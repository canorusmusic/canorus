/** @file drawable/drawablecontext.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "drawablecontext.h"

CADrawableContext::CADrawableContext(CAContext *c, int x, int y) : CADrawable(x, y) {
	_context = c;
}
