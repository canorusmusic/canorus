/*!
	Copyright (c) 2006, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/
 
#include "drawable/drawablerest.h"
#include "drawable/drawablecontext.h"
#include "drawable/drawablestaff.h"
#include "core/rest.h"

#include <QPainter>

CADrawableRest::CADrawableRest(CARest *rest, CADrawableContext *drawableContext, int x, int y)
 : CADrawableMusElement(rest, drawableContext, x, y) {
	_drawableMusElementType = CADrawableMusElement::DrawableRest;
	
	if (drawableContext->drawableContextType() != CADrawableContext::DrawableStaff)
		return;
	
	switch ( rest->playableLength().musicLength() ) {
	case CAPlayableLength::HundredTwentyEighth:
		_width = 16;
		_height = 49;
		_yPos = y;
		_xPos = x;
		break;
		
	case CAPlayableLength::SixtyFourth:
		_width = 14;
		_height = 41;
		_yPos = y;
		_xPos = x;
		break;
	
	case CAPlayableLength::ThirtySecond:
		_width = 12;
		_height = 33;
		_yPos = (int)(y + 2 + 0.5);
		_xPos = x;
		break;
		
	case CAPlayableLength::Sixteenth:
		_width = 10;
		_height = 24;
		_yPos = (int)(y + ((CADrawableStaff*)drawableContext)->lineSpace() + 0.5);
		_xPos = x;
		break;
		
	case CAPlayableLength::Eighth:
		_width = 8;
		_height = 17;
		_yPos = (int)(y + ((CADrawableStaff*)drawableContext)->lineSpace() + 0.5);
		_xPos = x;
		break;
		
	case CAPlayableLength::Quarter:
		_width = 8;
		_height = 20;
		_yPos = (int)(y + ((CADrawableStaff*)drawableContext)->lineSpace() + 0.5);
		_xPos = x;
		break;
	
	case CAPlayableLength::Half:
		_width = 12;
		_height = 5;
		_yPos = (int)(y + 1.5*((CADrawableStaff*)drawableContext)->lineSpace() + 0.5);	//values in constructor are the notehead center coords. yPos represents the top of the stem.
		_xPos = x;
		break;
		
	case CAPlayableLength::Whole:
		_width = 12;
		_height = 5;
		_yPos = (int)(y + ((CADrawableStaff*)drawableContext)->lineSpace() + 0.5);	//values in constructor are the notehead center coords. yPos represents the top of the stem.
		_xPos = x;
		break;
		
	case CAPlayableLength::Breve:
		_width = 4;
		_height = 9;
		_yPos = (int)(y + ((CADrawableStaff*)drawableContext)->lineSpace() + 0.5);
		_xPos = x;
		break;			
	}
	
	_restWidth = _width;
	
	if (rest->playableLength().dotted()) {
		_width += 3;
		for (int i=0; i<rest->playableLength().dotted(); i++)
			_width += 2;
	}
	
	_neededWidth = _width;
	_neededHeight = _height;

}

CADrawableRest::~CADrawableRest() {
}

CADrawableRest *CADrawableRest::clone(CADrawableContext* newContext) {
	return new CADrawableRest(rest(), (newContext)?newContext:_drawableContext, _xPos, _yPos);
}

void CADrawableRest::draw(QPainter *p, CADrawSettings s) {
	QFont font("Emmentaler");
	font.setPixelSize((int)(35*s.z));
	
	p->setPen(QPen(s.color));
	p->setFont(font);

	QPen pen;
	switch ( rest()->playableLength().musicLength() ) {
	case CAPlayableLength::HundredTwentyEighth: {
		p->drawText((int)(s.x + 4*s.z + 0.5), (int)(s.y + (2.6*((CADrawableStaff*)_drawableContext)->lineSpace())*s.z + 0.5), QString(0xE10D));
		break;
	}
	case CAPlayableLength::SixtyFourth: {
		p->drawText((int)(s.x + 3*s.z + 0.5), (int)(s.y + (1.75*((CADrawableStaff*)_drawableContext)->lineSpace())*s.z + 0.5), QString(0xE10C));
		break;
	}
	case CAPlayableLength::ThirtySecond: {
		p->drawText((int)(s.x + 2.5*s.z + 0.5), (int)(s.y + (1.8*((CADrawableStaff*)_drawableContext)->lineSpace())*s.z + 0.5), QString(0xE10B));
		break;
	}
	case CAPlayableLength::Sixteenth: {
		p->drawText((int)(s.x + 1*s.z + 0.5), (int)(s.y + (((CADrawableStaff*)_drawableContext)->lineSpace()-0.9)*s.z + 0.5), QString(0xE10A));
		break;
	}
	case CAPlayableLength::Eighth: {
		p->drawText(s.x, (int)(s.y + (((CADrawableStaff*)_drawableContext)->lineSpace()-0.9)*s.z + 0.5), QString(0xE109));
		break;
	}
	case CAPlayableLength::Quarter: {
		p->drawText(s.x,(int)(s.y + 0.5*height()*s.z),QString(0xE107));
		break;
	}
	case CAPlayableLength::Half: {
		p->drawText(s.x,(int)(s.y + height()*s.z + 0.5), QString(0xE101));
		break;
	}
	case CAPlayableLength::Whole: {
		p->drawText(s.x, s.y, QString(0xE100));
		break;
	}
	case CAPlayableLength::Breve: {
		p->drawText(s.x, (int)(s.y + height()*s.z + 0.5), QString(0xE106));
		break;
	}
	}
	
	///////////////
	// Draw Dots //
	///////////////
	float delta=4*s.z;
	for (int i=0; i<rest()->playableLength().dotted(); i++) {
		pen.setWidth((int)(2.7*s.z+0.5) + 1);
		pen.setCapStyle(Qt::RoundCap);
		pen.setColor(s.color);
		p->setPen(pen);
		p->drawPoint((int)(s.x + _restWidth*s.z + delta + 0.5), (int)(s.y + 0.3*_height*s.z + 0.5));
		delta += 3*s.z;
	}
	
}
