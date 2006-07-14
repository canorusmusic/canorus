/** @file drawablenote.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QPainter>

#include "drawablenote.h"
#include "note.h"

CADrawableNote::CADrawableNote(CANote *note, CADrawableContext *drawableContext, int x, int y)
 : CADrawableMusElement(note, drawableContext, x, y) {
	_drawableMusElement = CADrawableMusElement::DrawableNote;
	
	_width = 9;
	_height = 34;
	_neededWidth = _width;
	_neededHeight = _height;
}

void CADrawableNote::draw(QPainter *p, CADrawSettings s) {
#define QUARTER_STEM_LENGTH 20
	switch (note()->noteLength()) {
		case CANote::Quarter:
			s.y += (int)(QUARTER_STEM_LENGTH*s.z);
			p->setFont(QFont("Emmentaler",(int)(16*s.z)));
			p->drawText(s.x,s.y,QString(0xE125));
			QRect bBox = p->boundingRect(0,0,0,0,0,QString(0xE125));
			int w = bBox.width(); //get width from font metrics
			int h = (int)(0.5*w) + (int)(QUARTER_STEM_LENGTH*s.z);
			QPen pen;
			pen.setWidth((int)(1*s.z));
			pen.setCapStyle(Qt::RoundCap);
			pen.setColor(s.color);
			p->setPen(pen);
			p->drawLine((int)(s.x+w-0.5*s.z), (int)(s.y-1*s.z), (int)(s.x+w-0.5*s.z), s.y-(int)(QUARTER_STEM_LENGTH*s.z));
			
			break;
		
	}
}

CADrawableNote *CADrawableNote::clone() {
}
