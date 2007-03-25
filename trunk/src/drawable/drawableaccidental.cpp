/*!
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
 	_selectable = false;
 	
 	_width = 8;
 	_height = 14;
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
 	} else if (accs==2) {
 		_height = 6;
  		_yPos = y - _height/2;
 		_xPos = x;
 	} else if (accs==-2) {
  		_yPos = y - _height/2 - 5;
 		_xPos = x;
 		_width = 12;
 	}
 	
 	_centerX = x;
 	_centerY = y;
 	_neededWidth = _width;
 	_neededHeight = _height;
}

CADrawableAccidental::~CADrawableAccidental() {
}

void CADrawableAccidental::draw(QPainter *p, CADrawSettings s) {
	QFont font("Emmentaler");
	font.setPixelSize((int)(34*s.z));
	p->setPen(QPen(s.color));
	p->setFont(font);
	
	//Glyph unicode numbers for accidentals:
	//e10e - Emmentaler sharp
	//e112 - Emmentaler flat
	//e111 - Emmentaler natural
	//e116 - Emmentaler cross
	//e114 - Emmentaler double flat
	switch (_accs) {
		case 0:
			p->drawText(s.x, s.y + (int)(_height/2*s.z), QString(0xE111));
			break;
		case 1:
			p->drawText(s.x, s.y + (int)((_height/2 + 0.3)*s.z), QString(0xE10E));
			break;
		case -1:
			p->drawText(s.x, s.y + (int)((_height/2 + 5)*s.z), QString(0xE112));
			break;
		case 2:
			p->drawText(s.x, s.y + (int)(_height/2*s.z), QString(0xE116));
			break;
		case -2:
			p->drawText(s.x, s.y + (int)((_height/2 + 5)*s.z), QString(0xE114));
			break;
	}
}

CADrawableAccidental *CADrawableAccidental::clone(CADrawableContext* newContext) {
	return new CADrawableAccidental(_accs, _musElement, (newContext)?newContext:_drawableContext, _centerX, _centerY); 
}
