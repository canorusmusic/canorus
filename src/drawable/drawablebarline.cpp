/** @file drawablebarline.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QPen>
#include <QPainter>

#include "drawable/drawablebarline.h"
#include "drawable/drawablestaff.h"

#include "core/barline.h"

const float CADrawableBarline::BARLINE_WIDTH = 1.5;

CADrawableBarline::CADrawableBarline(CABarline *m, CADrawableStaff *staff, int x, int y)
 : CADrawableMusElement(m, staff, x, y) {
 	_drawableMusElementType = CADrawableMusElement::DrawableBarline;
 	
 	switch (m->barlineType()) {
 		case CABarline::Single:
 			_width = (int)(BARLINE_WIDTH + 1);
 			_height = staff->height();
 			break;
 	}

	_neededWidth = _width + 4;
	_neededHeight = _height;
}

CADrawableBarline::~CADrawableBarline() {
}

void CADrawableBarline::draw(QPainter *p, CADrawSettings s) {
	QPen pen;
	pen.setWidth((int)(BARLINE_WIDTH*s.z));
	pen.setCapStyle(Qt::FlatCap);
	pen.setColor(s.color);
	p->setPen(pen);
	
	p->drawLine((int)(s.x + (BARLINE_WIDTH*s.z)/2 + 0.5), s.y,
	            (int)(s.x + (BARLINE_WIDTH*s.z)/2 + 0.5), (int)(s.y + height()*s.z + 0.5));
}

CADrawableBarline* CADrawableBarline::clone(CADrawableContext* newContext) {
	return new CADrawableBarline((CABarline*)_musElement, (CADrawableStaff*)((newContext)?newContext:_drawableContext), _xPos, _yPos);
}
