/*!
   Copyright (c) 2007, Matevž Jekovec, Georg Rudolph, Canorus development team
   All Rights Reserved. See AUTHORS for a complete list of authors.

   Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef WRAPPER_H_
#define WRAPPER_H_


#ifdef __cplusplus
	extern "C" {
#endif

#define PMIDI_STATUS_END        0
#define PMIDI_STATUS_VERSION    1
#define PMIDI_STATUS_TEXT       2
#define PMIDI_STATUS_KEYSIG     3
#define PMIDI_STATUS_TIMESIG    4
#define PMIDI_STATUS_TEMPO      5
#define PMIDI_STATUS_NOTE       6
#define PMIDI_STATUS_DUMMY      7
#define PMIDI_STATUS_ROOT       8
#define PMIDI_STATUS_CONTROL    9
#define PMIDI_STATUS_PROGRAM    10
#define PMIDI_STATUS_PITCH      11
#define PMIDI_STATUS_PRESSURE   12
#define PMIDI_STATUS_KEYTOUCH   13
#define PMIDI_STATUS_SYSEX      14
#define PMIDI_STATUS_SMPTEOFFS  15

extern struct pmidi_outs {
	int format;       /* midi 1 or 2 */
	int tracks;
	int time_base;

	int micro_tempo;  /* micro secondes per quarter */

	int time;         /* occurence of element */
	int  type;        /* Type of text (lyric, copyright etc) */
	char *name;       /* Type as text */
	char *text;       /* actual text */

	int key;       /* Key signature */
	int minor;     /* Is this a minor key or not */

	int top;      /* 'top' of timesignature */
	int bottom;   /* 'bottom' of timesignature */
	int clocks;   /* Can't remember what this is */
	int n32pq;    /* Thirtysecond notes per quarter */

	int  program;   /* Program number */
	int  chan;	   /* Channel number */

	int  note;
	int  vel;
	int  length;
	int  offvel;	/* Note Off velocity */

	int  control;	/* Controller number */
	int  value;	    /* Controller value */

	int  pitch;     /* Pitch bending */

	int  hours;     /* SMPTE-Offset */
	int  minutes;
	int  seconds;
	int  frames;
	int  subframes;
} pmidi_out;

extern int pmidi_wrapper_status;
extern int pmidi_open_midi_file( const char *fileName );
extern int pmidi_parse_midi_file( void );

#ifdef __cplusplus
	}
#endif

#endif /* WRAPPER_H_ */

