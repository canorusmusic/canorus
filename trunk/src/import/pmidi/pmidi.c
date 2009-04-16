/*
 * 
 * Copyright (C) 1999-2003 Steve Ratcliffe
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 */


#include "glib.h"
#include "elements.h"
#include "except.h"
#include "intl.h"

#if HAVE_ALSA_ASOUNDLIB_H
#include <alsa/asoundlib.h>
#else
#include <sys/asoundlib.h>
#endif

#include <stdio.h>
#include "seqlib.h"
#include "md.h"
#include "midi.h"
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

/*
 * Play a midi file
 *	pmidi [-p client:port,... ] [-l] [-d delay] file ...
 *
 *	  -p client:port  - A alsa client and port number to send midi to
 *	  -l              - List possible output ports that could be used
 *	  -d delay        - Delay after song ends (default 2 sec)
 */


/* Options for the command */
#define HAS_ARG 1
static struct option long_opts[] = {
	{"port", HAS_ARG, NULL, 'p'},
	{"list", 0, NULL, 'l'},
	{"delay", HAS_ARG, NULL, 'd'},
	{"version", 0, NULL, 'v'},
	{0, 0, 0, 0},
};

/* Delay at the end of a song */
static int  delay = 2;

/* Pointer to root context */
seq_context_t *g_ctxp;

/* Number of elements in an array */
#define NELEM(a) ( sizeof(a)/sizeof((a)[0]) )

#define ADDR_PARTS 4 /* Number of part in a port description addr 1:2:3:4 */
#define SEP ", \t"	/* Separators for port description */

static seq_context_t *openports(char *portdesc);
static void playfile(seq_context_t *ctxp, char *filename);
static void showlist();
static void showusage();
static void play(void *arg, struct element *el);
static void no_errors_please(const char *file, int line, const char *function, int err, const char *fmt, ...);
static void set_signal_handler(seq_context_t *ctxp);
static void signal_handler(int sig);
static void showversion(void);

/*
 * Play a midi file
 *   pmidi [-p client:port ...] [-l] [-d delay] file ...
 *
 *   -p, --port=client:port  - An ALSA client and port number to use
 *   -l, --list              - List possible output ports that could be used
 *   -d, --delay=SECS        - Delay after song ends (default 2 sec)
 *   -v, --version           - Show version number
 *  Arguments:
 *    argc      - arg count
 *    argv      - arg vector
 */
int 
main(int argc, char **argv)
{
	char opts[NELEM(long_opts) * 2 + 1];
	char *portdesc;
	seq_context_t *ctxp;
	char *cp;
	int  c;
	struct option *op;

	/* Build up the short option string */
	cp = opts;
	for (op = long_opts; op < &long_opts[NELEM(long_opts)]; op++) {
		*cp++ = op->val;
		if (op->has_arg)
			*cp++ = ':';
	}

	/* Libaries shouldn't print errors by default! */
	snd_lib_error_set_handler(no_errors_please);

	portdesc = NULL;

	/* Deal with the options */
	for (;;) {
		c = getopt_long(argc, argv, opts, long_opts, NULL);
		if (c == -1)
			break;

		switch(c) {
		case 'p':
			portdesc = optarg;
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'l':
			showlist();
			exit(0);
			break;
		case 'v':
			showversion();
			exit(0);
		case '?':
			showusage();
			exit(1);
		}
	}

	if (portdesc == NULL) {
		portdesc = getenv("ALSA_OUTPUT_PORTS");
		/* Try the old name for the environment variable */
		if (portdesc == NULL)
			portdesc = getenv("ALSA_OUT_PORT");
		if (portdesc == NULL) {
			fprintf(stderr, "No client/port specified.\n"
				"You must supply one with the -p option or with the\n"
				"environment variable ALSA_OUTPUT_PORTS\n"
				);
			exit(1);
		}
	}

	ctxp = openports(portdesc);
	if (ctxp == NULL)
		return 1;

	/* Set signal handler */
	set_signal_handler(ctxp);

	/* Now play all the files */
	for (; optind < argc; optind++)
		playfile(ctxp, argv[optind]);

	seq_free_context(ctxp);

	/* Restore signal handler */
	signal(SIGINT, SIG_DFL);

	return 0;
}

/*
 * Read a list of client/port specifications and return an
 * array of snd_seq_addr_t that describes them.
 * 
 *  Arguments:
 *    portdesc  - 
 */
static seq_context_t *
openports(char *portdesc)
{
	char *astr;
	char *cp;
	seq_context_t *ctxp;
	snd_seq_addr_t *addr;
	snd_seq_addr_t *ap;
	int a[ADDR_PARTS];
	int count, naddr;
	int i;

	if (portdesc == NULL)
		return NULL;

	ctxp = seq_create_context();

	addr = g_new(snd_seq_addr_t, strlen(portdesc));

	naddr = 0;
	ap = addr;

	for (astr = strtok(portdesc, SEP); astr; astr = strtok(NULL, SEP)) {
		for (cp = astr, count = 0; cp && *cp; cp++) {
			if (count < ADDR_PARTS)
				a[count++] = atoi(cp);
			cp = strchr(cp, ':');
			if (cp == NULL)
				break;
		}

		switch (count) {
		case 2:
			ap->client = a[0];
			ap->port = a[1];
			break;
		default:
			printf("Addresses in %d parts not supported yet\n", count);
			break;
		}
		ap++;
		naddr++;
	}

	count = 0;
	for (i = 0; i < naddr; i++) {
		int  err;
		
		err = seq_connect_add(ctxp, addr[i].client, addr[i].port);
		if (err < 0) {
			fprintf(stderr, _("Could not connect to port %d:%d\n"),
				addr[i].client, addr[i].port);
		} else
			count++;
	}

	g_free(addr);

	if (count == 0) {
		seq_free_context(ctxp);
		return NULL;
	}

	return ctxp;
}

/*
 * Play a single file.
 *  Arguments:
 *    ctxp      - 
 *    filename  - 
 */
static void 
playfile(seq_context_t *ctxp, char *filename)
{
	struct rootElement *root;
	struct sequenceState *seq;
	struct element *el;
	unsigned long end;
	snd_seq_event_t *ep;

	if (strcmp(filename, "-") == 0)
		root = midi_read(stdin);
	else
		root = midi_read_file(filename);
	if (!root)
		return;

	/* Loop through all the elements in the song and play them */
	seq = md_sequence_init(root);
	while ((el = md_sequence_next(seq)) != NULL) {
		play(ctxp, el);
	}


	/* Get the end time for the tracks and echo an event to
	 * wake us up at that time
	 */
	end = md_sequence_end_time(seq);
	seq_midi_echo(ctxp, end);

#ifdef USE_DRAIN
	snd_seq_drain_output(seq_handle(ctxp));
#else
	snd_seq_flush_output(seq_handle(ctxp));
#endif

	/* Wait for all the events to be played */
	snd_seq_event_input(seq_handle(ctxp), &ep);

	/* Wait some extra time to allow for note to decay etc */
	sleep(delay);
	seq_stop_timer(ctxp);

	md_free(MD_ELEMENT(root));
}

/*
 * Show a list of possible output ports that midi could be sent
 * to.
 */
static void 
showlist()
{
	snd_seq_client_info_t *cinfo;
	snd_seq_port_info_t *pinfo;
	int  client;
	int  port;
	int  err;
	snd_seq_t *handle;

	err = snd_seq_open(&handle, "hw", SND_SEQ_OPEN_DUPLEX, 0);
	if (err < 0)
		except(ioError, _("Could not open sequencer %s"), snd_strerror(errno));

#ifdef USE_DRAIN
	/*
	 * NOTE: This is here so that it will give an error if the wrong
	 * version of alsa is used with USE_DRAIN set.  An incompatible change
	 * was made to ALSA on 29 Sep 2000 so that after that date you should
	 * define USE_DRAIN.
	 *
	 * Unfortunately with USE_DRAIN defined with an earlier version of alsa
	 * it will appear to compile fine but simply not work properly.  This
	 * unneeded call will give an error in this case.
	 *
	 * IF you see an error about this line, comment out the line beginning
	 * with USE_DRAIN in the make.conf file or compile with:
	 *    make USE_DRAIN=''
	 */
	snd_seq_drop_output(handle);
#endif

	snd_seq_client_info_alloca(&cinfo);
	snd_seq_client_info_set_client(cinfo, -1);
	printf(_(" Port     %-30.30s    %s\n"), _("Client name"), _("Port name"));

	while (snd_seq_query_next_client(handle, cinfo) >= 0) {
		client = snd_seq_client_info_get_client(cinfo);
		snd_seq_port_info_alloca(&pinfo);
		snd_seq_port_info_set_client(pinfo, client);

		snd_seq_port_info_set_port(pinfo, -1);
		while (snd_seq_query_next_port(handle, pinfo) >= 0) {
			int  cap;

			cap = (SND_SEQ_PORT_CAP_SUBS_WRITE|SND_SEQ_PORT_CAP_WRITE);
			if ((snd_seq_port_info_get_capability(pinfo) & cap) == cap) {
				printf("%3d:%-3d   %-30.30s    %s\n",
					snd_seq_port_info_get_client(pinfo),
					snd_seq_port_info_get_port(pinfo),
					snd_seq_client_info_get_name(cinfo),
					snd_seq_port_info_get_name(pinfo)
					);
			}
		}
	}
}

/*
 * Show a usage message
 */
static void 
showusage()
{
char **cpp;
static char *msg[] = {
N_("Usage: pmidi [-p client:port ...] [-l] [-d delay] file ..."),
"",
N_("  -p client:port  - A alsa client and port number to send midi to"),
N_("  -l              - List possible output ports that could be used"),
N_("  -d delay        - Delay after song ends (default 2 sec)"),
};

	for (cpp = msg; cpp < msg+NELEM(msg); cpp++) {
		fprintf(stderr, "%s\n", _(*cpp));
	}
}

static void
showversion()
{
	printf("pmidi-%s\n", VERSION);
}

static void 
play(void *arg, struct element *el)
{
	seq_context_t *ctxp = arg;
	snd_seq_event_t ev;
	
	seq_midi_event_init(ctxp, &ev, el->element_time, el->device_channel);
	switch (el->type) {
	case MD_TYPE_ROOT:
		seq_init_tempo(ctxp, MD_ROOT(el)->time_base, 120, 1);
		seq_start_timer(ctxp);
		break;
	case MD_TYPE_NOTE:
		seq_midi_note(ctxp, &ev, el->device_channel, MD_NOTE(el)->note, MD_NOTE(el)->vel,
			MD_NOTE(el)->length);
		break;
	case MD_TYPE_CONTROL:
		seq_midi_control(ctxp, &ev, el->device_channel, MD_CONTROL(el)->control,
			MD_CONTROL(el)->value);
		break;
	case MD_TYPE_PROGRAM:
		seq_midi_program(ctxp, &ev, el->device_channel, MD_PROGRAM(el)->program);
		break;
	case MD_TYPE_TEMPO:
		seq_midi_tempo(ctxp, &ev, MD_TEMPO(el)->micro_tempo);
		break;
	case MD_TYPE_PITCH:
		seq_midi_pitchbend(ctxp, &ev, el->device_channel, MD_PITCH(el)->pitch);
		break;
	case MD_TYPE_PRESSURE:
		seq_midi_chanpress(ctxp, &ev, el->device_channel, MD_PRESSURE(el)->velocity);
		break;
	case MD_TYPE_KEYTOUCH:
		seq_midi_keypress(ctxp, &ev, el->device_channel, MD_KEYTOUCH(el)->note,
			MD_KEYTOUCH(el)->velocity);
		break;
	case MD_TYPE_SYSEX:
		seq_midi_sysex(ctxp, &ev, MD_SYSEX(el)->status, MD_SYSEX(el)->data,
			MD_SYSEX(el)->length);
		break;
	case MD_TYPE_TEXT:
	case MD_TYPE_KEYSIG:
	case MD_TYPE_TIMESIG:
	case MD_TYPE_SMPTEOFFSET:
		/* Ones that have no sequencer action */
		break;
	default:
		printf("WARNING: play: not implemented yet %d\n", el->type);
		break;
	}
}

/**
 * alsa-lib has taken to printing errors on system call failures, even
 * though such failed calls are part of normal operation.  So we have to
 * install our own handler to shut it up and prevent it messing up the list
 * output.
 *
 * Possibly not needed any more Dec 2003.
 */
static void
no_errors_please(const char *file, int line,
		const char *function, int err, const char *fmt, ...)
{
}

static void
set_signal_handler(seq_context_t *ctxp)
{
	struct sigaction *sap = calloc(1, sizeof(struct sigaction));

	g_ctxp = ctxp;

	sap->sa_handler = signal_handler;
	sigaction(SIGINT, sap, NULL);
}

/* signal handler */
static void
signal_handler(int sig)
{
	/* Close device */
	if (g_ctxp) {
		seq_free_context(g_ctxp);
	}

	exit(1);
}
