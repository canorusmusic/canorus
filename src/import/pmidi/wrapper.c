#include "glib.h"
#include "elements.h"

#include <stdio.h>
#include "midi.h"
#include "md.h"
//#include "seqlib.h"
#include "wrapper.h"


int pmidi_wrapper_status = PMIDI_STATUS_END;
struct pmidi_outs pmidi_out;

/* pmidi's structures while parsing the file
*/
static struct rootElement *root;
static struct sequenceState *seq;
static unsigned long end;
//static snd_seq_event_t *ep;

/*
	Transfer the name of the to be imported midi file to pmidi,
	so it can opened here too.
*/
int pmidi_open_midi_file( const char *name )
{
	printf(" In pmidi ist der Filename angekommen: %s\n", name );
	pmidi_wrapper_status = PMIDI_STATUS_END;
	root = midi_read_file((char *)name);
	seq = md_sequence_init(root);
	return root ? 0 : 1;
}

char str[256];

/*
	Example for a callback function to bring data back to the calling party.
*/
int pmidi_parse_midi_file( void )
{
	/* this worked to give a string back:
	static int x = 0;
	sprintf( str, "  Ausgabe von PMIDI: %d\n", x );
	*p = str;
	x++;
	return pmidi_main( NULL );
	*/

	switch (pmidi_wrapper_status) {
	case PMIDI_STATUS_END:
		pmidi_out.time = 0;
		pmidi_out.format = root->format;
		pmidi_out.tracks = root->tracks;
		pmidi_out.time_base = root->time_base;
		pmidi_wrapper_status = PMIDI_STATUS_DUMMY;
		return PMIDI_STATUS_VERSION;
	}
	struct element *el;
	while ((el = md_sequence_next(seq)) != NULL) {
////////////////////////////////////

		pmidi_out.time = el->element_time;

		//snd_seq_event_t ev;
		//static struct seq_context_t *ctxp;
	
		/* resulted in a crash, seems to to be needed:
		seq_midi_event_init(ctxp, &ev, el->element_time, el->device_channel);
		*/
	
		printf("Hmm %2d   \n", el->type );
	
		switch (el->type) {
		case MD_TYPE_ROOT:
			//seq_init_tempo(ctxp, MD_ROOT(el)->time_base, 120, 1);
			//seq_start_timer(ctxp);
			pmidi_out.format    = MD_ROOT(el)->format;
			pmidi_out.tracks    = MD_ROOT(el)->tracks;
			pmidi_out.time_base = MD_ROOT(el)->time_base;
			// Default tempo
			pmidi_out.micro_tempo = 60000000 / 120;
			return PMIDI_STATUS_ROOT;
		case MD_TYPE_NOTE:
			//printf("  at time: %6d   ", el->element_time );
			//seq_midi_note(ctxp, &ev, el->device_channel, MD_NOTE(el)->note, MD_NOTE(el)->vel,
			//	MD_NOTE(el)->length);
			pmidi_out.chan   = el->device_channel;
			pmidi_out.note   = MD_NOTE(el)->note;
			pmidi_out.vel    = MD_NOTE(el)->vel;
			pmidi_out.length = MD_NOTE(el)->length;
			return PMIDI_STATUS_NOTE;
		case MD_TYPE_CONTROL:
			//printf("  at time: %6d   ", el->element_time );
			pmidi_out.chan = el->device_channel;
			pmidi_out.control = MD_CONTROL(el)->control;
			pmidi_out.value = MD_CONTROL(el)->value;
			return PMIDI_STATUS_CONTROL;
		case MD_TYPE_PROGRAM:
			//printf("  at time: %6d   ", el->element_time );
			//seq_midi_program(ctxp, &ev, el->device_channel, MD_PROGRAM(el)->program);
			return PMIDI_STATUS_PROGRAM;
		case MD_TYPE_TEMPO:
			//printf("  at time: %6d   ", el->element_time );
			//seq_midi_tempo(ctxp, &ev, MD_TEMPO(el)->micro_tempo);
			pmidi_out.micro_tempo  = MD_TEMPO(el)->micro_tempo;
			return PMIDI_STATUS_TEMPO;
		case MD_TYPE_PITCH:
			//seq_midi_pitchbend(ctxp, &ev, el->device_channel, MD_PITCH(el)->pitch);
			return PMIDI_STATUS_PITCH;
		case MD_TYPE_PRESSURE:
			//seq_midi_chanpress(ctxp, &ev, el->device_channel, MD_PRESSURE(el)->velocity);
			return PMIDI_STATUS_PRESSURE;
		case MD_TYPE_KEYTOUCH:
			//seq_midi_keypress(ctxp, &ev, el->device_channel, MD_KEYTOUCH(el)->note,
			//	MD_KEYTOUCH(el)->velocity);
			return PMIDI_STATUS_KEYTOUCH;
		case MD_TYPE_SYSEX:
			//seq_midi_sysex(ctxp, &ev, MD_SYSEX(el)->status, MD_SYSEX(el)->data,
			//	MD_SYSEX(el)->length);
			return PMIDI_STATUS_SYSEX;
		case MD_TYPE_KEYSIG:
			//seq_midi_keysig(ctxp, &ev, MD_KEYSIG(el)->key, MD_KEYSIG(el)->minor);
			pmidi_out.key      = MD_KEYSIG(el)->key;
			pmidi_out.minor    = MD_KEYSIG(el)->minor;
			return PMIDI_STATUS_KEYSIG;
		case MD_TYPE_TEXT:
			//seq_midi_text(ctxp, &ev, MD_TEXT(el)->type, MD_TEXT(el)->name,
			//	MD_TEXT(el)->text, MD_TEXT(el)->length);
			return PMIDI_STATUS_TEXT;
		case MD_TYPE_TIMESIG:
			//seq_midi_timesig(ctxp, &ev, MD_TIMESIG(el)->top, MD_TIMESIG(el)->bottom,
			//	MD_TIMESIG(el)->clocks, MD_TIMESIG(el)->n32pq);
			pmidi_out.top       = MD_TIMESIG(el)->top;
			pmidi_out.bottom    = MD_TIMESIG(el)->bottom;
			pmidi_out.clocks    = MD_TIMESIG(el)->clocks;
			pmidi_out.n32pq     = MD_TIMESIG(el)->n32pq;

			return PMIDI_STATUS_TIMESIG;
		case MD_TYPE_SMPTEOFFSET:
			//printf("\n");
			/* Ones that have no sequencer action */
			pmidi_out.hours = MD_SMPTEOFFSET(el)->hours;
			pmidi_out.minutes = MD_SMPTEOFFSET(el)->minutes;
			pmidi_out.seconds = MD_SMPTEOFFSET(el)->seconds;
			pmidi_out.frames = MD_SMPTEOFFSET(el)->frames;
			pmidi_out.subframes = MD_SMPTEOFFSET(el)->subframes;
			return PMIDI_STATUS_SMPTEOFFS;
		default:
			printf("WARNING: play: not implemented yet %d\n", el->type);
			return PMIDI_STATUS_DUMMY;
		}
////////////////////////////////////
	}

	/* Get the end time for the tracks and echo an event to
	 * wake us up at that time
	 */
	// FIXME end = md_sequence_end_time(seq);
	// FIXME seq_midi_echo(ctxp, end);

#ifdef USE_DRAIN
	// FIXME snd_seq_drain_output(seq_handle(ctxp));
#else
	// FIXME snd_seq_flush_output(seq_handle(ctxp));
#endif

	/* Wait for all the events to be played */
	// FIXME snd_seq_event_input(seq_handle(ctxp), &ep);

	/* Wait some extra time to allow for note to decay etc */
	// FIXME sleep(delay);
	// FIXME seq_stop_timer(ctxp);

	md_free(MD_ELEMENT(root));

	/*
	printf("In Root:\n Parent-Type %d\n Parent-Device-Channel: %d\n",
		root->parent->elements[0]->type, root->parent->elements[0]->device_channel );
	*/
	return PMIDI_STATUS_END;
}

