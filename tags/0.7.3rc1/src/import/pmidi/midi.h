/*
 * 
 * File: midi.h
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
 * 
 * 
 */
/*
 * Midi status byte values.
 */
#define MIDI_NOTE_OFF	0x80
#define MIDI_NOTE_ON	0x90
#define MIDI_KEY_AFTERTOUCH	0xa0
#define MIDI_CONTROLER	0xb0
#define MIDI_PATCH	0xc0
#define MIDI_CHANNEL_AFTERTOUCH	0xd0
#define MIDI_PITCH_WHEEL	0xe0
#define MIDI_SYSEX	0xf0
#define MIDI_META	0xff

/* Meta event defines */
#define MIDI_META_SEQUENCE   0
/* The text type meta events */
#define MIDI_META_TEXT       1
#define MIDI_META_COPYRIGHT  2
#define MIDI_META_TRACKNAME  3
#define MIDI_META_INSTRUMENT 4
#define MIDI_META_LYRIC      5
#define MIDI_META_MARKER     6
#define MIDI_META_CUE        7
/* More meta events */
#define MIDI_META_CHANNEL      0x20
#define MIDI_META_PORT         0x21
#define MIDI_META_EOT          0x2f
#define MIDI_META_TEMPO        0x51
#define MIDI_META_SMPTE_OFFSET 0x54
#define MIDI_META_TIME         0x58
#define MIDI_META_KEY          0x59
#define MIDI_META_PROP         0x7f

/** The maximum of the midi defined text types */
#define MIDI_MAX_TEXT_TYPE    7

#define MIDI_HEAD_MAGIC 0x4d546864
#define MIDI_TRACK_MAGIC  0x4d54726b

struct rootElement *midi_read(FILE *fp);
struct rootElement *midi_read_file(char *name);
