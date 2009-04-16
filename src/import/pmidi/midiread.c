/*
 * 
 * File: midiread.m - Read in a midi file.
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

#include <stdio.h>
#include "midi.h"

/*
 * This structure is used to keep track of the state while
 * reading in a midi file.
 */
struct midistate {
	FILE *fp;		/* File being read */
	int  current_time;	/* Current midi time */
	int  port;		/* Midi port number */
	int  device;	/* Midi device number */
	int  track_count;	/* Count of current track */
	int  chunk_size;	/* Size of current chunk */
	int  chunk_count;	/* Count within current chunk */
	GPtrArray *notes;	/* Currently on notes */

	struct tempomapElement *tempo_map;	/* The tempo map */
};

static struct rootElement *read_head(struct midistate *msp);
static struct trackElement *read_track(struct midistate *msp);
static void handle_status(struct midistate *msp, struct trackElement *track, 
        int status);
static struct metaElement *handle_meta(struct midistate *msp, int type, 
        unsigned char *data);
static int read_int(struct midistate *msp, int n);
static unsigned char *read_data(struct midistate *msp, int length);
static gint32 read_var(struct midistate *msp);
static void put_back(struct midistate *msp, char c);
static struct element *save_note(struct midistate *msp, int note, int vel);
static void finish_note(struct midistate *msp, int note, int vel);
static void skip_chunk(struct midistate *msp);

/*
 * Read in a midi file from the specified open file pointer, fp
 * and return an mtree structure tree representing the file.
 * 
 *  Arguments:
 *    fp        - Input file pointer
 */
struct rootElement *
midi_read(FILE *fp)
{
	struct midistate mState;
	struct midistate *msp;
	struct rootElement *root;
	struct element *el;
	int  i;

	msp = &mState;
	msp->fp = fp;
	msp->tempo_map = md_tempomap_new();
	msp->notes = g_ptr_array_new();
	msp->port = 0;

	root = read_head(msp);
	md_add(MD_CONTAINER(root), NULL); /* Leave room for the tempo map */
	for (i = 0; i < root->tracks; i++) {
		el = MD_ELEMENT(read_track(msp));

		/* If format 1 then the first track is really the tempo map */
		if (root->format == 1
				&& i == 0
				&& MD_CONTAINER(el)->elements->len == 0) {
			/* It will be added after the loop */
			md_free(el);
			continue;
		}

		md_add(MD_CONTAINER(root), el);
	}

	g_ptr_array_index(MD_CONTAINER(root)->elements, 0) = msp->tempo_map;
	msp->tempo_map = NULL;

	g_ptr_array_free(msp->notes, 1);

	return root;
}

/*
 * Read in a midi file from the specified file name.
 * 
 *  Arguments:
 *    name      - File name to read
 */
struct rootElement *
midi_read_file(char *name)
{
	FILE *fp;
	struct rootElement *root;

	fp = fopen(name, "rb");
	if (fp == NULL)
		except(ioError, _("Could not open file %s"), name);

	root = midi_read(fp);

	fclose(fp);

	return root;
}

/*
 * Read the header information from a midi file
 * 
 *  Arguments:
 *    msp       - current midi state
 */
static struct rootElement *
read_head(struct midistate *msp)
{
	guint32  magic;
	int  length;
	struct rootElement *root;

	root = md_root_new();

	/* The first word just identifies the file as a midi file */
	magic = read_int(msp, 4);
	if (magic != MIDI_HEAD_MAGIC)
		except(formatError, _("Bad header (%x), probably not a real midi file"),
			magic);

	/* The header chunk should be 6 bytes, (perhaps longer in the future) */
	length = read_int(msp, 4);
	if (length < 6)
		except(formatError, _("Bad header length, probably not a real midi file"));

	root->format = read_int(msp, 2);
	root->tracks = read_int(msp, 2);
	root->time_base = read_int(msp, 2);

	/* Should skip any extra bytes, (may not be seekable) */
	while (length > 6) {
		length--;
		(void) getc(msp->fp);
	}

	return root;
}

/*
 * Read in one track from the file, and return an element tree
 * describing it.
 * 
 *  Arguments:
 *    msp       - Midi state
 */
static struct trackElement *
read_track(struct midistate *msp)
{
	int  status, laststatus;
	int  head;
	int  length;
	int  delta_time;
	struct trackElement *track;
	int  i;

	laststatus = 0;
	head = read_int(msp, 4);
	if (head != MIDI_TRACK_MAGIC)
		except(formatError,
			_("Bad track header (%x), probably not a midi file"),
			head);

	length = read_int(msp, 4);
	msp->chunk_size = length;
	msp->chunk_count = 0;	/* nothing read yet */

	track = md_track_new();

	msp->current_time = 0;
	while (msp->chunk_count < msp->chunk_size) {

		delta_time = read_var(msp);
		msp->current_time += delta_time;

		status = read_int(msp, 1);
		if ((status & 0x80) == 0) {
			
			/*
			 * This is not a status byte and so running status is being
			 * used.  Re-use the previous status and push back this byte.
			 */
			put_back(msp, status);
			status = laststatus;
		} else {
			laststatus = status;
		}

		handle_status(msp, track, status);
	}

  restart:
	for (i = 0; i < msp->notes->len; i++) {
		struct noteElement *ns;
		ns = g_ptr_array_index(msp->notes, i);
		msp->device = MD_ELEMENT(ns)->device_channel;
printf("Left over note, finishing\n");
		finish_note(msp, ns->note, 0);
		goto restart;
	}

	msp->track_count++;

	return track;
}

/*
 * Complete the reading of the status byte. The parsed midi
 * command will be added to the specified track .
 * 
 *  Arguments:
 *    msp       - Current midi file status
 *    track     - Current track
 *    status    - Status byte, ie. current command
 */
static void 
handle_status(struct midistate *msp, struct trackElement *track, int status)
{
	int  ch;
	int  type;
	int  device;
	int  length;
	short note, vel, control;
	int  val;
	unsigned char *data;
	struct element *el;

	ch = status & 0x0f;
	type = status & 0xf0;

	/*
	 * Do not set the device if the type is 0xf0 as these commands are
	 * not channel specific
	 */
	device = msp->port<<4;
	if (type != 0xf0)
		device += ch;
	msp->device = device;

	el = NULL;

	switch (type) {	
	case MIDI_NOTE_OFF:
		note = read_int(msp, 1);
		vel = read_int(msp, 1);

		finish_note(msp, note, vel);
		break;

	case MIDI_NOTE_ON:
		note = read_int(msp, 1);
		vel = read_int(msp, 1);

		if (vel == 0) {
			/* This is really a note off */
			finish_note(msp, note, vel);
		} else {
			/* Save the start, so it can be matched with the note off */
			el = save_note(msp, note, vel);
		}
		break;

	case MIDI_KEY_AFTERTOUCH:
		note = read_int(msp, 1);
		vel = read_int(msp, 1);

		/* new aftertouchElement */
		el = MD_ELEMENT(md_keytouch_new(note, vel));
		break;

	case MIDI_CONTROLER:
		control = read_int(msp, 1);
		val = read_int(msp, 1);
		el = MD_ELEMENT(md_control_new(control, val));

		break;
	
	case MIDI_PATCH:
		val = read_int(msp, 1);
		el = MD_ELEMENT(md_program_new(val));
		break;

	case MIDI_CHANNEL_AFTERTOUCH:
		val = read_int(msp, 1);
		el = MD_ELEMENT(md_pressure_new(val));
		break;
	case MIDI_PITCH_WHEEL:
		val = read_int(msp, 1);
		val |= read_int(msp, 1) << 7;
		val -= 0x2000;	/* Center it around zero */
		el = MD_ELEMENT(md_pitch_new(val));
		break;

	/* Now for all the non-channel specific ones */
	case 0xf0:
		/* Deal with the end of track event first */
		if (ch == 0x0f) {
			type = read_int(msp, 1);
			if (type == 0x2f) {
				/* End of track - skip to end of real track */
				track->final_time = msp->current_time;
				skip_chunk(msp);
				return;
			}
		}

		/* Get the length of the following data */
		length = read_var(msp);
		data = read_data(msp, length);
		if (ch == 0x0f) {
			el = (struct element *)handle_meta(msp, type, data);
		} else {
			el = (struct element *)md_sysex_new(status, data, length);
		}
		break;
	default:
		except(formatError, _("Bad status type 0x%x"), type);
		/*NOTREACHED*/
	}

	if (el != NULL) {
		el->element_time = msp->current_time;
		el->device_channel = device;

		md_add(MD_CONTAINER(track), el);
	}
}

/*
 * Do extra handling of meta events. We want to save time
 * signature and key for use elsewhere, for example. This
 * routine create the correct type of class and returns it.
 * 
 *  Arguments:
 *    msp       - The midi file state
 *    type      - The meta event type
 *    data      - The data for the event
 */
static struct metaElement *
handle_meta(struct midistate *msp, int type, unsigned char *data)
{
	struct metaElement *el = NULL;
	struct mapElement *map = NULL;
	int  micro_tempo;

	switch (type) {
	case MIDI_META_SEQUENCE:
		break;
	case MIDI_META_TEXT:
	case MIDI_META_COPYRIGHT:
	case MIDI_META_TRACKNAME:
	case MIDI_META_INSTRUMENT:
	case MIDI_META_LYRIC:
	case MIDI_META_MARKER:
	case MIDI_META_CUE:
		/* Text based events */
		el = MD_META(md_text_new(type, (char*)data));
		break;
	case MIDI_META_CHANNEL:
		break;
	case MIDI_META_PORT:
		msp->port = data[0];
		g_free(data);
		break;
	case MIDI_META_EOT:
		break;
	case MIDI_META_TEMPO:
		micro_tempo = ((data[0]<<16) & 0xff0000)
			+ ((data[1]<<8) & 0xff00) + (data[2] & 0xff);
		map = MD_MAP(md_tempo_new(micro_tempo));
		g_free(data);
		break;
	case MIDI_META_SMPTE_OFFSET:
		el = MD_META(md_smpteoffset_new(data[0], data[1], data[2], data[3],
			data[4]));
		break;
	case MIDI_META_TIME:
		map = MD_MAP(md_timesig_new(data[0], 1<<data[1],
			data[2], data[3]));
		g_free(data);
		break;
	case MIDI_META_KEY:
		map = MD_MAP(md_keysig_new(data[0], (data[1]==1)? 1: 0));
		g_free(data);
		break;
	case MIDI_META_PROP:
		/* Proprietry sequencer specific event */
		/* Just throw it out */
		break;
	default:
		g_warning(_("Meta event %d not implemented\n"), type);
		break;
	}

	/* If this affected the tempo map then add it */
	if (map) {
		MD_ELEMENT(map)->element_time = msp->current_time;
		md_add(MD_CONTAINER(msp->tempo_map), MD_ELEMENT(map));
	}

	return el;
}

/*
 * Reads an interger from the midi file. The number of bytes to
 * be read is specified in n .
 * 
 *  Arguments:
 *    msp       - Midi file state
 *    n         - Number of bytes to read
 */
static int 
read_int(struct midistate *msp, int n)
{
	int  val;
	int  c;
	int  i;

	val = 0;

	for (i = 0; i < n; i++) {
		val <<= 8;
		c = getc(msp->fp);
		msp->chunk_count++;
		if (c == -1)
			except(formatError, _("Unexpected end of file"));

		val |= c;
	}

	return val;
}

/*
 * Read in a specified amount of data from the file. The return
 * is allocated data which must be freed by the caller. An extra
 * null byte is appended for the sake of text events.
 *  Arguments:
 *    msp       - Midifile state
 *    length    - Length of data to read
 */
static unsigned char *
read_data(struct midistate *msp, int length)
{

	unsigned char *data = g_malloc(length+1);

	if (length == 0) {
		data[0] = 0;
		return data;
	}

	if (fread(data, length, 1, msp->fp) == 1) {
		msp->chunk_count += length;
		data[length] = '\0';
		return data;
	} else {
		except(formatError, _("Unexpected end of file"));
		/*NOTREACHED*/
	}
	return NULL;
}

/*
 * Read a variable length integer from the midifile and return
 * it. Returns an int32 so cannot really deal with more than
 * four bytes.
 * 
 *  Arguments:
 *    msp       - Midi file state
 */
static gint32 
read_var(struct midistate *msp)
{
	int  val;
	int  c;

	val = 0;
	do {
		c = getc(msp->fp);
		msp->chunk_count++;
		if (c == -1)
			except(formatError, _("Unexpected end of file"));
		val <<= 7;
		val |= (c & 0x7f);
	} while ((c & 0x80) == 0x80);

	return val;
}

/*
 * Push back a character. Have to also keep track of the
 * chunk_count.
 *  Arguments:
 *    msp       - Midi input state
 *    c         - Character to push back
 */
static void 
put_back(struct midistate *msp, char c)
{
	ungetc(c, msp->fp);
	msp->chunk_count--;
}

/*
 * Save the initial note-on message. This will later be paired
 * with a note off message. We have to keep track of channel and
 * device that the note-on is for so that it can be correctly
 * matched.
 * 
 *  Arguments:
 *    msp       - Midi file state
 *    note      - Note number
 *    vel       - Velocity
 */
static struct element *
save_note(struct midistate *msp, int note, int vel)
{
	struct noteElement *n;

	/* Create a new note and set its length to -1
	 * this will be filled in later, when the note-off arrives
	 */
	n = md_note_new(note, vel, -1); 

	/* Save it so that we match up with the note off */
	g_ptr_array_add(msp->notes, n);

	return MD_ELEMENT(n);
}

/*
 * Called when a note off is seen. Finds the corresponding note
 * on and constructs a note element.
 * 
 *  Arguments:
 *    msp       - Midi file state
 *    note      - Note number
 *    vel       - Note velocity
 */
static void 
finish_note(struct midistate *msp, int note, int vel)
{
	int  i;
	GPtrArray *notes;
	struct noteElement *n;
	int  len;

	notes = msp->notes;
	n = NULL;
	for (i = notes->len-1; i >= 0; i--) {
		n = g_ptr_array_index(notes, i);
		if (n->note == note && MD_ELEMENT(n)->device_channel == msp->device) {
			len = msp->current_time - MD_ELEMENT(n)->element_time;
			n->offvel = vel;
			n->length = len;
			if (n->length < 0) {
				printf("Len neg: msp->time%d, s->time%d, note=%d, s.vel%d\n",
					msp->current_time, MD_ELEMENT(n)->element_time,
					note, n->vel);
				n->length = 0;
			}
			g_ptr_array_remove_index_fast(notes, i);
			break;
		}
	}
}

/*
 * Skip to the end of the chunk. Note that the input may not be
 * seekable, so we just read bytes until at the end of the chunk.
 * 
 *  Arguments:
 *    msp       - Midi file state
 */
static void 
skip_chunk(struct midistate *msp)
{
	while (msp->chunk_count < msp->chunk_size)
		(void) read_int(msp, 1);
}
