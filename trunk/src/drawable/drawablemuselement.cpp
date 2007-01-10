/** @file drawable/drawablemuselement.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "drawable/drawablemuselement.h"

CADrawableMusElement::CADrawableMusElement(CAMusElement *m, CADrawableContext *drawableContext, int x, int y)
 : CADrawable(x, y) {
 	_drawableType = CADrawable::DrawableMusElement;
	_musElement = m;
	_drawableContext = drawableContext;
	_selectable = true;
}
