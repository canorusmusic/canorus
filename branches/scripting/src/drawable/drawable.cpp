/** @file drawable.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QPainter>

#include "drawable.h"

CADrawable::CADrawable(int x, int y) {
	_xPos = x;
	_yPos = y;
	_xPosOffset = 0;
	_yPosOffset = 0;
	_visible = true;
	_selectable = true;
}
