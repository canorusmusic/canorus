/*
 * intl.h - Internationalisation for melys
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

#ifdef I18N

#include "libintl.h"

#define _(_STRING) gettext(_STRING)
#define N_(_STRING) _STRING

#else

#define _(_STRING) _STRING
#define N_(_STRING) _STRING

#endif
