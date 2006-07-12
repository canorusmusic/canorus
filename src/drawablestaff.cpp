/** @file drawablestaff.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QPainter>

#include "drawablestaff.h"
#include "staff.h"

CADrawableStaff::CADrawableStaff(CAStaff *s, int x, int y) : CADrawable(x, y) {
	_staff = s;
	_width = 0;
	_height = 60;
}

void CADrawableStaff::draw(QPainter *p, const CADrawSettings s) {
	QPen pen;
	pen.setWidth((int)(1*s.z));
	pen.setCapStyle(Qt::RoundCap);
	pen.setColor(s.color);
	p->setPen(pen);
	
	int dy=0;
	for (int i=0; i<_staff->numberOfLines(); i++, dy = (int)(((float)i/(_staff->numberOfLines()-1)) * _height * s.z)) {
		p->drawLine(0, s.y + dy,
		            s.w, s.y + dy);
	}
}

CADrawableStaff *CADrawableStaff::clone() {
	CADrawableStaff *d = new CADrawableStaff(_staff, _xPos, _yPos);
	
	return d;
}
