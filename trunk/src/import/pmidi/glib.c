/*
 * File: glib.c Replacements for glib functions used by pmidi
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
 * Code is derived from garray.c which is copyright:
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 */

#include "glib.h"

/*
 * There is no need to use this file, linking against glib is preferable
 * if you have it.
 */

#define MIN_ARRAY_SIZE  16


static gint
g_nearest_pow (gint num)
{
  gint n = 1;

  while (n < num)
    n <<= 1;

  return n;
}

/* Pointer Array
 */
typedef struct _GRealPtrArray  GRealPtrArray;

struct _GRealPtrArray
{
  gpointer *pdata;
  guint     len;
  guint     alloc;
};


GPtrArray*
g_ptr_array_new(void)
{
GRealPtrArray *array;

	array = g_new(GRealPtrArray, 1);

	array->pdata = NULL;
	array->len = 0;
	array->alloc = 0;

	return (GPtrArray*) array;
}

void
g_ptr_array_free(GPtrArray   *array, gboolean  free_segment)
{

	if (free_segment)
		g_free(array->pdata);

	g_free(array);
}

static void
g_ptr_array_maybe_expand(GRealPtrArray *array, gint len)
{
	guint old_alloc;

	if ((array->len + len) > array->alloc)
	{
		old_alloc = array->alloc;

		array->alloc = g_nearest_pow(array->len + len);
		array->alloc = MAX(array->alloc, MIN_ARRAY_SIZE);
		if (array->pdata) {
			array->pdata = g_realloc(array->pdata,
				sizeof(gpointer) * array->alloc);
		} else {
			array->pdata = g_new0(gpointer, array->alloc);
		}

		memset(array->pdata + old_alloc, 0, array->alloc - old_alloc);
	}
}


gpointer
g_ptr_array_remove_index_fast(GPtrArray* farray, guint index)
{
	GRealPtrArray* array = (GRealPtrArray*)farray;
	gpointer result;


	result = array->pdata[index];

	if (index != array->len - 1)
		array->pdata[index] = array->pdata[array->len - 1];

	array->pdata[array->len - 1] = NULL;

	array->len -= 1;

	return result;
}



typedef struct _GRealArray  GRealArray;

struct _GRealArray
{
  guint8 *data;
  guint   len;
  guint   alloc;
  guint   elt_size;
  guint   zero_terminated : 1;
  guint   clear : 1;
};

void
g_ptr_array_add(GPtrArray* farray, gpointer data)
{
	GRealPtrArray* array = (GRealPtrArray*) farray;

	g_ptr_array_maybe_expand(array, 1);

	array->pdata[array->len++] = data;
}

static void
g_array_maybe_expand (GRealArray *array, gint len)
{
	guint want_alloc = (array->len + len + array->zero_terminated) * array->elt_size;
	if (want_alloc > array->alloc) {
		guint old_alloc = array->alloc;

		array->alloc = g_nearest_pow (want_alloc);
		array->alloc = MAX(array->alloc, MIN_ARRAY_SIZE);

		array->data = g_realloc (array->data, array->alloc);

		if (array->clear || array->zero_terminated)
			memset (array->data + old_alloc, 0, array->alloc - old_alloc);
	}
}

GArray*
g_array_new (gboolean zero_terminated, gboolean clear, guint elt_size)
{
	GRealArray *array;

	array = g_new(GRealArray, 1);

	array->data            = NULL;
	array->len             = 0;
	array->alloc           = 0;
	array->zero_terminated = (zero_terminated ? 1 : 0);
	array->clear           = (clear ? 1 : 0);
	array->elt_size        = elt_size;

	return (GArray*) array;
}

void
g_array_free (GArray  *array, gboolean free_segment)
{
	if (free_segment)
		g_free (array->data);

	g_free(array);
}

GArray*
g_array_append_vals(GArray *farray, gconstpointer data, guint len)
{
	GRealArray *array = (GRealArray*) farray;

	g_array_maybe_expand (array, len);

	memcpy (array->data + array->elt_size * array->len, data, array->elt_size * len);

	array->len += len;

	return farray;
}
