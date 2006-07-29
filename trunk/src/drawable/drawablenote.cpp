/** @file drawablenote.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QPainter>
#include <iostream>
#include "drawable/drawablenote.h"
#include "core/voice.h"
#include "core/staff.h"
#include "core/note.h"

#define QUARTER_STEM_LENGTH 22
#define QUARTER_YPOS_DELTA 21

CADrawableNote::CADrawableNote(CANote *note, CADrawableContext *drawableContext, int x, int y, bool shadowNote)
 : CADrawableMusElement(note, drawableContext, x, y) {
	_drawableMusElement = CADrawableMusElement::DrawableNote;
	
	switch (note->noteLength()) {
		case CANote::Quarter:
			_width = 11;
			_height = 26;
			_yPos = y - QUARTER_YPOS_DELTA;	//values in constructor are the notehead center coords. yPos represents the top of the stem.
			_xPos = x - _width/2;
			_neededWidth = _width;
			_neededHeight = _height;
			break;
			
		case CANote::Whole:
			_width = 16;
			_height = 3;
			_yPos = y - _height/2;
			_xPos = x - _width/2;
			_neededWidth = _width;
			_neededHeight = _height;
			break;			
	}
	
	_shadowNote = shadowNote;
	
	_drawLedgerLines = true;
}

CADrawableNote::~CADrawableNote() {
}

void CADrawableNote::draw(QPainter *p, CADrawSettings s) {
	QFont font("Emmentaler");
	font.setPixelSize((int)(35*s.z));
	
	p->setPen(QPen(s.color));
	p->setFont(font);

	QPen pen;
	switch (note()->noteLength()) {
		case CANote::Quarter:
			//draw notehead
			s.y += (int)(QUARTER_STEM_LENGTH*s.z);
			p->drawText(s.x,(int)(s.y - 0.1*s.z),QString(0xE125));

			//draw stem
			pen.setWidth((int)(1.2*s.z));
			pen.setCapStyle(Qt::RoundCap);
			pen.setColor(s.color);
			p->setPen(pen);
			p->drawLine((int)(s.x + (_width - 0.2)*s.z), (int)(s.y-1*s.z), (int)(s.x + (_width - 0.2)*s.z), s.y-(int)(QUARTER_STEM_LENGTH*s.z));
						
			break;
			
		case CANote::Whole:
			//draw notehead
			s.y += (int)((_height*s.z)/2);
			p->drawText(s.x, (int)(s.y), QString(0xE123));

			break;
	}
	
	//draw ledger lines
	if ( _drawLedgerLines &&
	     note() && note()->voice() && note()->voice()->staff() &&
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
	switch (note()->noteLength()) {
		case CANote::Quarter:
			return new CADrawableNote(note(), drawableContext(), _xPos + _width/2, _yPos + QUARTER_YPOS_DELTA);
			break;
		case CANote::Whole:
			return new CADrawableNote(note(), drawableContext(), _xPos + _width/2, _yPos + _height/2);
			break;		
	}
}

void CADrawableNote::setXPos(int xPos) {
	switch (note()->noteLength()) {
		case CANote::Quarter:
			_xPos = xPos - _width/2;
			break;
		case CANote::Whole:
			_xPos = xPos - _width/2;
			break;
	}
}

void CADrawableNote::setYPos(int yPos) {
	switch (note()->noteLength()) {
		case CANote::Quarter:
			_yPos = yPos - QUARTER_YPOS_DELTA;
			break;
		case CANote::Whole:
			_yPos = yPos - _height/2;
			break;
	}
}
