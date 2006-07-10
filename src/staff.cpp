/** @file staff.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QPainter>
#include <iostream>
#include "voice.h"
#include "staff.h"

CAStaff::CAStaff(CASheet *s, int x, int y) : CAContext(s, x, y) {
	_numberOfLines = 5;
	_height = 60;
	_width = 0;
}

void CAStaff::draw(QPainter *p, const CADrawSettings s) {
	QPen pen;
	pen.setWidth((int)(1*s.z));
	pen.setCapStyle(Qt::RoundCap);
	p->setPen(pen);
	
	int dy=0;
	for (int i=0; i<_numberOfLines; i++, dy = (int)(((float)i/(_numberOfLines-1)) * _height * s.z)) {
		p->drawLine(0, s.y + dy,
		            s.w, s.y + dy);
	}
}

void CAStaff::clear() {
	for (int i=0; i<_voiceList.size(); i++) {
		_voiceList[i]->clear();
		delete _voiceList[i];
	}
	
	_voiceList.clear();
}
