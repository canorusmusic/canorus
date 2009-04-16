/*
 * 
 * except.m - XXX
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

#include <setjmp.h>

struct except {
	int  set;
	sigjmp_buf  buf;
};

extern struct except *formatError;	/* Bad file format */
extern struct except *ioError;	/* Error reading/writing file */
extern struct except *debugError;	/* Debugging 'shouldn't happen' errors */

void except(struct except *e, char *message, ...);
