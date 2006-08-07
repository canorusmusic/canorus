/** @file drawablebarline.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "drawable/drawablebarline.h"
#include "drawable/drawablestaff.h"

CADrawableBarline::CADrawableBarline(CAMusElement *m, CADrawableStaff *staff, int x, int y)
 : CADrawableMusElement(m, staff, x, y) {
 	_drawableMusElement = CADrawableMusElement::DrawableBarline;
}

CADrawableBarline::~CADrawableBarline() {
}
