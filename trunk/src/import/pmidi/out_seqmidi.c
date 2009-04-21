/*
 * File: seqmidi.m - convert to the sequencer events
 * 
 * Copyright (C) 1999 Steve Ratcliffe
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */


#include "glib.h"
#include "elements.h"
#include "except.h"
#include "intl.h"

#include <string.h>
#include "seqlib.h"
#include "seqpriv.h"


/*
 * Start the timer. (What about timers other than the system
 * one?)
 */
void 
seq_start_timer(seq_context_t *ctxp)
{
	// FIXME seq_control_timer(ctxp, SND_SEQ_EVENT_START);
}


/*
 * Set the initial time base and tempo. This should only be used
 * for initialisation when there is nothing playing. To
 * change the tempo during a song tempo change events are used.
 * If realtime is false the resolution is in ticks per quarter
 * note. If true, the the resolution is microseconds. There is
 * a macro XXX to convert from SMPTE codes.
 * 
 *  Arguments:
 *    ctxp      - Application context
 *    resolution - Ticks per quarter note or realtime resolution
 *    tempo     - Beats per minute
 *    realtime  - True if absolute time base
 */
int 
seq_init_tempo(seq_context_t *ctxp, int resolution, int tempo, int realtime)
{
/* FIXME
	snd_seq_queue_tempo_t *qtempo;
	int  ret;

	snd_seq_queue_tempo_alloca(&qtempo);
	// FIXME memset(qtempo, 0, snd_seq_queue_tempo_sizeof());
	// FIXME snd_seq_queue_tempo_set_ppq(qtempo, resolution);
	snd_seq_queue_tempo_set_tempo(qtempo, 60*1000000/tempo);

	ret = snd_seq_set_queue_tempo(ctxp->handle, ctxp->queue, qtempo);

	return ret;
*/
	printf("    Init Tempo, resolution %d, tempo %d, realtime %d\n");
	return 0;
}

/*
 * Catch the time signature
 * Parameters:
 *		top			'top' of timesignature
 *		bottom      'bottom' of timesignature
 *		clocks		Can't remember what this is
 *		n32pq		Thirtysecond notes per quarter
*/
void 
seq_midi_timesig(seq_context_t *ctxp, snd_seq_event_t *ep, int top, int bottom, int clocks, int n32pq)
{
	printf("   Time signature: Top %d, Bottom %d, Clocks %d, Thirtysecond notes per quarter: %d\n",
		top, bottom, clocks, n32pq );
}


/*
 * Write out the event. This routine blocks until
 * successfully written. 
 * 
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event
 */
int 
seq_write(seq_context_t *ctxp, snd_seq_event_t *ep)
{
	int  err = 0;
	printf("   seq_write() reached. Type: %d Time: %d\n", ep);
	return err;

	err = snd_seq_event_output(ctxp->handle, ep);
	if (err < 0)
		return err;

	return err;
}



/*
 * Initialize a midi event from the context. The source and
 * destination addresses will be set from the information
 * stored in the context, which depends on the previous
 * connection calls. In addition the time and channel are set.
 * This should be called first before any of the following
 * functions.
 * 
 *  Arguments:
 *    ctxp      - Client application
 *    ep        - Event to init
 *    time      - Midi time
 *    devchan   - Midi channel
 */
void 
seq_midi_event_init(seq_context_t *ctxp, snd_seq_event_t *ep, 
        unsigned long time, int devchan)
{
	int  dev;

	dev = devchan >> 4;

	/*
	 * If insufficient output devices have been registered, then we
	 * just scale the device back to fit in the correct range.  This
	 * is not necessarily what you want.
	 */
	if (dev >= ctxp->ctxndest)
		dev = dev % ctxp->ctxndest;
	
	snd_seq_ev_clear(ep);
	snd_seq_ev_schedule_tick(ep, ctxp->queue, 0, time);
	ep->source = ctxp->source;
	if (ctxp->ctxndest > 0)
		ep->dest = g_array_index(ctxp->destlist, snd_seq_addr_t, dev);
}

/*
 * Send a note event.
 *  Arguments:
 *    ctxp      - Client context
 *    ep        - Event template
 *    note      - Pitch of note
 *    vel       - Velocity of note
 *    length    - Length of note
 */
void 
seq_midi_note(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int note, int vel, 
        int length)
{
	ep->type = SND_SEQ_EVENT_NOTE;

	ep->data.note.channel = devchan & 0xf;
	ep->data.note.note = note;
	ep->data.note.velocity = vel;
	ep->data.note.duration = length;
	
	printf(" A note in channel %4d pitch %4d velocity %4d and length %4d\n",
	ep->data.note.channel,
	ep->data.note.note,
	ep->data.note.velocity,
	ep->data.note.duration );

	// seq_write(ctxp, ep);
}

/*
 * Send a note on event.			Not in use!!
 *  Arguments:
 *    ctxp      - Client context
 *    ep        - Event template
 *    note      - Pitch of note
 *    vel       - Velocity of note
 *    length    - Length of note
 */
void 
seq_midi_note_on(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int note, int vel, 
        int length)
{
	ep->type = SND_SEQ_EVENT_NOTEON;

	ep->data.note.channel = devchan & 0xf;
	ep->data.note.note = note;
	ep->data.note.velocity = vel;
	ep->data.note.duration = length;

	seq_write(ctxp, ep);
}

/*
 * Send a note off event.
 *  Arguments:
 *    ctxp      - Client context
 *    ep        - Event template
 *    note      - Pitch of note
 *    vel       - Velocity of note
 *    length    - Length of note
 */
void 
seq_midi_note_off(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int note, int vel, 
        int length)
{
	ep->type = SND_SEQ_EVENT_NOTEOFF;

	ep->data.note.channel = devchan & 0xf;
	ep->data.note.note = note;
	ep->data.note.velocity = vel;
	ep->data.note.duration = length;

	seq_write(ctxp, ep);
}

/*
 * Send a key pressure event.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    note      - Note to be altered
 *    value     - Pressure value
 */
void 
seq_midi_keypress(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int note, 
        int value)
{
	ep->type = SND_SEQ_EVENT_KEYPRESS;

	ep->data.control.channel = devchan & 0xf;
	ep->data.control.param = note;
	ep->data.control.value = value;
	seq_write(ctxp, ep);
}

/*
 * Send a control event.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    control   - Controller to change
 *    value     - Value to set it to
 */
void 
seq_midi_control(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int control, 
        int value)
{
	ep->type = SND_SEQ_EVENT_CONTROLLER;

	ep->data.control.channel = devchan & 0xf;
	ep->data.control.param = control;
	ep->data.control.value = value;
	printf("                          control: %d channel %d value: %d\n",
	ep->data.control.param,
	ep->data.control.channel,
	ep->data.control.value );
	// seq_write(ctxp, ep);
}

/*
 * Send a program change event.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    program   - Program to set
 */
void 
seq_midi_program(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int program)
{
	ep->type = SND_SEQ_EVENT_PGMCHANGE;

	ep->data.control.channel = devchan & 0xf;
	ep->data.control.value = program;
	printf("                      new program: %d channel %d\n",
	ep->data.control.value,
	ep->data.control.channel );
	// seq_write(ctxp, ep);
}

/*
 * Send a channel pressure event.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    pressure  - Pressure value
 */
void 
seq_midi_chanpress(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int pressure)
{
	ep->type = SND_SEQ_EVENT_CHANPRESS;

	ep->data.control.channel = devchan & 0xf;
	ep->data.control.value = pressure;
	seq_write(ctxp, ep);
}

/*
 * Catch the text messages
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    text      - Type, type as text, text, length (including 0?)
 */
void 
seq_midi_text(seq_context_t *ctxp, snd_seq_event_t *ep, int type, char *name, char *text, int length)
{
	printf("       text type: %d in words: %s, length %d\n",
		type, name, length );
		printf("                                                                                   %s\n", text );
}


/*
 * Catch the keysignature event.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    bend      - Bend value, centered on zero.
 */
void 
seq_midi_keysig(seq_context_t *ctxp, snd_seq_event_t *ep, int key, int minor)
{
	printf("                  keysig: %d minor %d\n",
		key, minor );
}

/*
 * Send a pitchbend message. The bend parameter is centered on
 * zero, negative values mean a lower pitch.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    bend      - Bend value, centered on zero.
 */
void 
seq_midi_pitchbend(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int bend)
{
	ep->type = SND_SEQ_EVENT_PITCHBEND;

	ep->data.control.channel = devchan & 0xf;
	ep->data.control.value = bend;
	seq_write(ctxp, ep);
}

/*
 * Send a tempo event. The tempo parameter is in microseconds
 * per beat.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    tempo     - New tempo in usec per beat
 */
void 
seq_midi_tempo(seq_context_t *ctxp, snd_seq_event_t *ep, int tempo)
{
	ep->type = SND_SEQ_EVENT_TEMPO;

	ep->data.queue.queue = ctxp->queue;
	ep->data.queue.param.value = tempo;
	ep->dest.client = SND_SEQ_CLIENT_SYSTEM;
	ep->dest.port = SND_SEQ_PORT_SYSTEM_TIMER;
	printf("    new tempo: %d\n", tempo );
	// seq_write(ctxp, ep);
}

/*
 * Send a sysex event. The status byte is to distiguish
 * continuation sysex messages.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    status    - Status byte for sysex
 *    data      - Data to send
 *    length    - Length of data
 */
void 
seq_midi_sysex(seq_context_t *ctxp, snd_seq_event_t *ep, int status, 
        unsigned char *data, int length)
{
	unsigned char *ndata;
	int  nlen;

	ep->type = SND_SEQ_EVENT_SYSEX;

	ndata = g_malloc(length + 1);
	nlen = length +1;

	ndata[0] = status;
	memcpy(ndata+1, data, length);

	snd_seq_ev_set_variable(ep, nlen, ndata);

	seq_write(ctxp, ep);

	g_free(ndata);
}

/*
 * Send an echo event back to the source client at the specified
 * time.
 * 
 *  Arguments:
 *    ctxp      - Application context
 *    time      - Time of event
 */
void 
seq_midi_echo(seq_context_t *ctxp, unsigned long time)
{
	snd_seq_event_t ev;

	seq_midi_event_init(ctxp, &ev, time, 0);
	/* Loop back */
	ev.dest = ctxp->source;
	seq_write(ctxp, &ev);
}

