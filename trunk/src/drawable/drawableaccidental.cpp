/** @file drawableaccidental.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QFont>
#include <QPainter>

#include "drawable/drawableaccidental.h"
#include "core/muselement.h"
#include "drawable/drawablecontext.h"
#include "drawable/drawableclef.h"

CADrawableAccidental::CADrawableAccidental(signed char accs, CAMusElement *musElement, CADrawableContext *drawableContext, int x, int y)
 : CADrawableMusElement(musElement, drawableContext, x, y) {
 	_drawableMusElementType = CADrawableMusElement::DrawableAccidental;
 	
 	_centerX = x + _width/2;
 	_centerY = y;
 	_width = 8;
 	_height = 15;
 	_accs = accs;
 	
 	if (accs==0) {
 		_yPos = y - _height/2;
 		_xPos = x;
 	} else if (accs==1) {
 		_yPos = y - _height/2;
 		_xPos = x;
 	} else if (accs==-1) {
  		_yPos = y - _height/2 - 5;
 		_xPos = x;
 	}
}

CADrawableAccidental::~CADrawableAccidental() {
}

void CADrawableAccidental::draw(QPainter *p, CADrawSettings s) {
	QFont font("Emmentaler");
	font.setPixelSize((int)(35*s.z));
	p->setPen(QPen(s.color));
	p->setFont(font);
	
	//Glyph unicode numbers for accidentals:
	//e10e - Emmentaler sharp
	//e112 - Emmentaler flat
	//e111 - Emmentaler natural
	//e116 - Emmentaler cross
	//e114 - Emmentaler double cross
	
	switch (_accs) {
		case 0:
			p->drawText(s.x, s.y + (int)(_height/2*s.z), QString(0xE111));
			break;
		case 1:
			p->drawText(s.x, s.y + (int)(_height/2*s.z), QString(0xE10E));
			break;
		case -1:
			p->drawText(s.x, s.y + (int)((_height/2 + 5)*s.z), QString(0xE112));
			break;
	}
}

CADrawableAccidental *CADrawableAccidental::clone() {
	return new CADrawableAccidental(_accs, _musElement, _drawableContext, _centerX, _centerY); 
}
