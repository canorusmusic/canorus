/*
 * 
 * File: elements.m - Operation on all elements
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

#include "md.h"

static void md_container_init(struct containerElement *e);

/*
 * Create and initialise a element element.
 */
struct element *
md_element_new()
{
	struct element *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_ELEMENT;
	return new;
}

/*
 * Create and initialise a container element.
 */
struct containerElement *
md_container_new()
{
	struct containerElement *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_CONTAINER;
	return new;
}

/*
 * Initialize data structures within the element. This is
 * only really required when pointers need allocating.
 *  Arguments:
 *    e         - Element to init
 */
static void 
md_container_init(struct containerElement *e)
{

	e->elements = g_ptr_array_new();
}

/*
 * Create and initialise a root element.
 *  Arguments:
 *              - 
 */
struct rootElement *
md_root_new(void)
{
	struct rootElement *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_ROOT;
	md_container_init(MD_CONTAINER(new));
	return new;
}

/*
 * Create and initialise a track element.
 *  Arguments:
 *              - 
 */
struct trackElement *
md_track_new(void)
{
	struct trackElement *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_TRACK;
	md_container_init(MD_CONTAINER(new));
	return new;
}

/*
 * Create and initialise a tempomap element.
 */
struct tempomapElement *
md_tempomap_new()
{
	struct tempomapElement *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_TEMPOMAP;
	md_container_init(MD_CONTAINER(new));
	return new;
}

/*
 * Create and initialise a note element.
 *  Arguments:
 *    note      - 
 *    vel       - 
 *    length    - 
 */
struct noteElement *
md_note_new(short note, short vel, int length)
{
	struct noteElement *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_NOTE;
	new->note = note;
	new->vel  = vel;
	new->length = length;
	new->offvel = 0;
	return new;
}

/*
 * Create and initialise a part element.
 *  Arguments:
 *              - 
 */
struct partElement *
md_part_new(void)
{
	struct partElement *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_PART;
	md_container_init(MD_CONTAINER(new));
	return new;
}

/*
 * Create and initialise a control element.
 *  Arguments:
 *    control   - 
 *    value     - 
 */
struct controlElement *
md_control_new(short control, short value)
{
	struct controlElement *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_CONTROL;
	new->control = control;
	new->value = value;
	return new;
}

/*
 * Create and initialise a program element.
 *  Arguments:
 *    program   - 
 */
struct programElement *
md_program_new(int program)
{
	struct programElement *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_PROGRAM;
	new->program = program;
	return new;
}

/*
 * Create and initialise a keytouch element.
 *  Arguments:
 *    note      - 
 *    vel       - 
 */
struct keytouchElement *
md_keytouch_new(int note, int vel)
{
	struct keytouchElement *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_KEYTOUCH;
	new->note = note;
	new->velocity = vel;
	return new;
}

/*
 * Create and initialise a pressure element.
 *  Arguments:
 *    vel       - 
 */
struct pressureElement *
md_pressure_new(int vel)
{
	struct pressureElement *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_PRESSURE;
	new->velocity = vel;
	return new;
}

/*
 * Create and initialise a pitch element.
 */
struct pitchElement *
md_pitch_new(int val)
{
	struct pitchElement *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_PITCH;
	new->pitch = val;
	return new;
}

/*
 * Create and initialise a sysex element.
 */
struct sysexElement *
md_sysex_new(int status, unsigned char *data, int len)
{
	struct sysexElement *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_SYSEX;
	new->status = status;
	new->data = data;
	new->length = len;
	return new;
}

/*
 * Create and initialise a meta element.
 */
struct metaElement *
md_meta_new()
{
	struct metaElement *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_META;
	return new;
}

/*
 * Create and initialise a map element.
 */
struct mapElement *
md_map_new()
{
	struct mapElement *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_MAP;
	return new;
}

/*
 * Create and initialise a keysig element.
 */
struct keysigElement *
md_keysig_new(short key, short minor)
{
	struct keysigElement *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_KEYSIG;
	new->key = key;
	new->minor = minor != 0? 1: 0;
	return new;
}

/*
 * Create and initialise a timesig element.
 */
struct timesigElement *
md_timesig_new(short top, short bottom, short clocks, short n32pq)
{
	struct timesigElement *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_TIMESIG;
	new->top = top;
	new->bottom = bottom;
	new->clocks = clocks;
	new->n32pq = n32pq;
	return new;
}

/*
 * Create and initialise a tempo element.
 */
struct tempoElement *
md_tempo_new(int m)
{
	struct tempoElement *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_TEMPO;
	new->micro_tempo = m;
	return new;
}

/*
 * Create and initialise a text element.
 */
struct textElement *
md_text_new(int type, char *text)
{
	struct textElement *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_TEXT;
	{
	static char *typenames[] = {
		"",
		"text",
		"copyright",
		"trackname",
		"instrument",
		"lyric",
		"marker",
		"cuepoint",
	};
	new->type = type;
	new->name = typenames[type];
	new->text = text;
	if (text)
		new->length = strlen(text);
	else
		new->length = 0;
	}
	return new;
}

/*
 * Create and initialise a smpteoffset element.
 */
struct smpteoffsetElement *
md_smpteoffset_new(short hours, short minutes, short seconds, short frames, 
        short subframes)
{
	struct smpteoffsetElement *  new;

	new = g_malloc0(sizeof(*new));
	MD_ELEMENT(new)->type = MD_TYPE_SMPTEOFFSET;
	new->hours = hours;
	new->minutes = minutes;
	new->seconds = seconds;
	new->frames = frames;
	new->subframes = subframes;
	return new;
}

/*
 * Add an element to a container element.
 */
void 
md_add(struct containerElement *c, struct element *e)
{
	g_ptr_array_add(c->elements, e);
}

/*
 * Free a complete element tree.
 */
void 
md_free(struct element *el)
{
	struct containerElement *c;
	int  i;
	
	if (el->type >= MD_CONTAINER_BEGIN) {
		c = MD_CONTAINER(el);
		for (i = 0; i < c->elements->len; i++) {
			struct element *p = g_ptr_array_index(c->elements, i);
			md_free(p);
		}
		g_ptr_array_free(c->elements, 1);
	}
	switch (el->type) {
	case MD_TYPE_TEXT:
		g_free(MD_TEXT(el)->text);
		break;
	case MD_TYPE_SYSEX:
		g_free(MD_SYSEX(el)->data);
		break;
	}
	g_free(el);

}

/*
 * Check that the given element can be casted to the given type.
 * This is mainly for debugging as mismatches will not happen
 * in proper use. In particular do not use this routine to check
 * types unless you want the program to exit if the check fails.
 *  Arguments:
 *    el        - Element to be cast
 *    type      - type to cast to
 */
struct element *
md_check_cast(struct element *el, int type)
{

	switch (type) {
	case MD_TYPE_CONTAINER:
		if (!iscontainer(el))
			except(debugError, "Cast to container from %d", el->type);
		return el;
	case MD_TYPE_ELEMENT:
		/* Anything can be cast to an element */
		if (el->type > 100 || el->type < 0)
			break;	/* Sanity check */
		return el;
	case MD_TYPE_META:
	case MD_TYPE_MAP:
		/* TEMP: this is a parent type */
		return el;
	case MD_TYPE_TRACK:
		if (el->type == MD_TYPE_TEMPOMAP)
			return el;
		break;
	}

	if (type == el->type)
		return el;

	except(debugError, "Cast from %d to %d not allowed", el->type, type);
	return NULL;/* not reached */

}

