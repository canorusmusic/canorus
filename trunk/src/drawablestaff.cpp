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
#include "note.h"
#include "clef.h"

#define _lineSpace ((float)_height/staff()->numberOfLines())

CADrawableStaff::CADrawableStaff(CAStaff *s, int x, int y) : CADrawableContext(s, x, y) {
	_width = 0;
	_height = 40;
}

void CADrawableStaff::draw(QPainter *p, const CADrawSettings s) {
	QPen pen;
	pen.setWidth((int)(1*s.z));
	pen.setCapStyle(Qt::RoundCap);
	pen.setColor(s.color);
	p->setPen(pen);
	
	int dy=0;
	for (int i=0; i<staff()->numberOfLines(); i++, dy = (int)(((float)i/(staff()->numberOfLines()-1)) * _height * s.z)) {
		p->drawLine(0, s.y + dy,
		            s.w, s.y + dy);
	}
}

CADrawableStaff *CADrawableStaff::clone() {
	CADrawableStaff *d = new CADrawableStaff(staff(), _xPos, _yPos);
	
	return d;
}

int CADrawableStaff::calculateCenterYCoord(CANote *note, CAClef *clef) {
	return (int)( (yPos() + height() -
	               //c' in logical pitch is 28
	               ((note->pitch() - 28) - (clef?clef->c1():-2)) 
	              )*(_lineSpace/2)
	            );
}

int CADrawableStaff::calculateCenterYCoord(int y) {
	float newY = (yPos() - y) / (_lineSpace/2);
	newY += 0.5;	//round
	newY = (float)((int)newY);	// "
	
	return (int)(yPos() - ((newY+1) * (_lineSpace/2)));
}
