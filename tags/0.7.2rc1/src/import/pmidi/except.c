/*
 * 
 * except.m - error handling code
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

#include <stdarg.h>
#include "stdio.h"


static struct except format;
static struct except io;
static struct except debug;

struct except *formatError = &format;	/* Bad file format */
struct except *ioError = &io;	/* I/o error to file */
struct except *debugError = &debug;	/* Debugging 'shouldn't happen' errors */

/*
 * Deal with errors. At present just exit, will allow error
 * recovery in the future.
 *  Arguments:
 *    e         - Exception thrown
 *    message   - Message
 *              - Takes variable number of arguments
 */
void 
except(struct except *e, char *message, ...)
{
	va_list ap;

	va_start(ap, message);
	vfprintf(stderr, message, ap);
	va_end(ap);
	putc('\n', stderr);

#ifdef DEBUG
	g_on_error_stack_trace("a.out");
#endif
	exit(1);
}

