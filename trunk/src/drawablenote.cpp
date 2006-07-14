/** @file drawablenote.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QPainter>
#include <iostream>
#include "drawablenote.h"
#include "voice.h"
#include "staff.h"
#include "note.h"

CADrawableNote::CADrawableNote(CANote *note, CADrawableContext *drawableContext, int x, int y)
 : CADrawableMusElement(note, drawableContext, x, y) {
	_drawableMusElement = CADrawableMusElement::DrawableNote;
	
	_width = 10;
	_height = 26;
	_yPos = y - 19;	//values in constructor are the notehead center coords. yPos represents the top of the stem.
	_neededWidth = _width;
	_neededHeight = _height;
	
	_drawLedgerLine = true;
}

void CADrawableNote::draw(QPainter *p, CADrawSettings s) {
#define QUARTER_STEM_LENGTH 22
	switch (note()->noteLength()) {
		case CANote::Quarter:
			p->setPen(QPen(s.color));
			
			//draw notehead
			p->setFont(QFont("Emmentaler",(int)(19*s.z)));
			s.y += (int)(QUARTER_STEM_LENGTH*s.z);
			p->drawText(s.x,(int)(s.y - 0.1*s.z),QString(0xE125));

			//draw stem
			QPen pen;
			pen.setWidth((int)(1.2*s.z));
			pen.setCapStyle(Qt::RoundCap);
			pen.setColor(s.color);
			p->setPen(pen);
			p->drawLine((int)(s.x + (_width - 0.4)*s.z), (int)(s.y-1*s.z), (int)(s.x + (_width - 0.4)*s.z), s.y-(int)(QUARTER_STEM_LENGTH*s.z));
						
			break;
	}
	
	//draw ledger lines
	if ( _drawLedgerLine &&
	     ( (note()->notePosition() <= -2) ||	//note is below the staff
	       (note()->notePosition() >= note()->voice()->staff()->numberOfLines()*2)	//note is above the staff
	     )
	   ) {
	   	int direction = (note()->notePosition() > 0 ? 1 : -1);	//1 falling, -1 rising
	   	int ledgerDist = (int)(9.0*s.z);	//distance between the ledger lines - notehead height
	   	
	   	//draw ledger lines in direction from the notehead to staff
	   	int y = (int)(s.y-0.6*s.z);	//initial y
	   	if ((note()->notePosition() % 2) != 0)	//if the note is not on the ledger line, shift the ledger line by half space
	   		y += (ledgerDist/2)*direction;

		QPen pen(s.color);
		pen.setWidth((int)(1.0*s.z));
		p->setPen(pen);
	   	for (int i=0;
	   	     i < ((note()->notePosition()*direction -
	   	          ((direction>0)?((note()->voice()->staff()->numberOfLines()-1)*2):0))/2);
	   	     ++i, y += (ledgerDist*direction)
	   	    )
			p->drawLine((int)(s.x - 4*s.z), (int)(y), (int)(s.x + (_width + 4)*s.z), (int)(y));
	}
	
}

CADrawableNote *CADrawableNote::clone() {
	return new CADrawableNote(note(), drawableContext(), _xPos, _yPos+19);
}
