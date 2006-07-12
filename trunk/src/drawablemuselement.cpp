/** @file drawablemuselement.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "drawablemuselement.h"

CADrawableMusElement::CADrawableMusElement(CAMusElement *m, int x, int y) : CADrawable(x, y) {
	_musElement = m;
}
