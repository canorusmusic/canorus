/*!
	Copyright (c) 2006-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPainter>
#include <iostream>
#include "drawable/drawablenote.h"
#include "drawable/drawablecontext.h"
#include "drawable/drawableaccidental.h"
#include "core/voice.h"
#include "core/staff.h"

const int CADrawableNote::HUNDREDTWENTYEIGHTH_STEM_LENGTH = 43;
const int CADrawableNote::SIXTYFOURTH_STEM_LENGTH = 43;
const int CADrawableNote::THIRTYSECOND_STEM_LENGTH = 37;
const int CADrawableNote::SIXTEENTH_STEM_LENGTH = 31;
const int CADrawableNote::EIGHTH_STEM_LENGTH = 31;
const int CADrawableNote::QUARTER_STEM_LENGTH = 31;
const int CADrawableNote::HALF_STEM_LENGTH = 31;
const int CADrawableNote::QUARTER_YPOS_DELTA = 21;
const int CADrawableNote::HALF_YPOS_DELTA = 23;

/*!
	Default constructor.
	
	\param x coordinate represents the left border of the notehead.
	\param y coordinate represents the center of the notehead.
*/
CADrawableNote::CADrawableNote(CANote *n, CADrawableContext *drawableContext, int x, int y, bool shadowNote, CADrawableAccidental *drawableAcc)
 : CADrawableMusElement(n, drawableContext, x, y) {
	_drawableMusElementType = CADrawableMusElement::DrawableNote;
	_drawableAcc = drawableAcc;
	
	_stemDirection = note()->actualStemDirection();
	
	// Notehead widths are hardcoded below; it's possible to determine them at runtime using QFontMetrics, if necessary.
	switch (n->playableLength().musicLength()) {
	case CAPlayableLength::HundredTwentyEighth:
	case CAPlayableLength::SixtyFourth:
	case CAPlayableLength::ThirtySecond:
	case CAPlayableLength::Sixteenth:
	case CAPlayableLength::Eighth:
	case CAPlayableLength::Quarter:
		_noteHeadCodepoint = 0xE125;
		_penWidth = 1.2;
		_width = 11;
		_height = 10;
		break;
	
	case CAPlayableLength::Half:
		_noteHeadCodepoint = 0xE124;
		_penWidth = 1.3;
		_width = 12;
		_height = 10;
		break;
		
	case CAPlayableLength::Whole:
		_noteHeadCodepoint = 0xE123;
		_penWidth = 0;
		_width = 17;
		_height = 8;
		break;
	
	case CAPlayableLength::Breve:
		_noteHeadCodepoint = 0xE122;
		_penWidth = 0;
		_width = 18;
		_height = 8;
		break;
	}
	_yPos = (int)(y - _height/2.0 + 0.5);
	_xPos = x;

	switch (n->playableLength().musicLength()) {
	case CAPlayableLength::HundredTwentyEighth:
		/// \todo Emmentaler font doesn't have 128th, 64th flag is drawn instead! Need to somehow compose the 128th flag? -Matevz
		_stemLength = HUNDREDTWENTYEIGHTH_STEM_LENGTH;
		_stemUpCodepoint = 0xE18A;
		_stemDownCodepoint = 0xE190;
		break;
	case CAPlayableLength::SixtyFourth:
		_stemLength = SIXTYFOURTH_STEM_LENGTH;
		_stemUpCodepoint = 0xE18A;
		_stemDownCodepoint = 0xE190;
		break;
	case CAPlayableLength::ThirtySecond:
		_stemLength = THIRTYSECOND_STEM_LENGTH;
		_stemUpCodepoint = 0xE189;
		_stemDownCodepoint = 0xE18F;
		break;
	case CAPlayableLength::Sixteenth:
		_stemLength = SIXTEENTH_STEM_LENGTH;
		_stemUpCodepoint = 0xE188;
		_stemDownCodepoint = 0xE18E;
		break;
	case CAPlayableLength::Eighth:
		_stemLength = EIGHTH_STEM_LENGTH;
		_stemUpCodepoint = 0xE187;
		_stemDownCodepoint = 0xE18B;
		break;
	case CAPlayableLength::Quarter:
		_stemLength = QUARTER_STEM_LENGTH;
		_stemUpCodepoint = _stemDownCodepoint = 0;
		break;
	case CAPlayableLength::Half:
		_stemLength = HALF_STEM_LENGTH;
		_stemUpCodepoint = _stemDownCodepoint = 0;
		break;
	case CAPlayableLength::Whole:
	case CAPlayableLength::Breve:
		_stemLength = _stemUpCodepoint = _stemDownCodepoint = 0;
		break;
	}
	
	_noteHeadWidth = _width;
	
	if (n->playableLength().dotted()) {
		_width += 3;
		for (int i=0; i<n->playableLength().dotted(); i++)
			_width += 2;
	}
	
	_neededWidth = _width;
	_neededHeight = _height;
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
	
	// Draw ledger lines
	if ( _drawLedgerLines &&
	     note() && note()->voice() && note()->voice()->staff() &&
	     ( (note()->notePosition() <= -2) ||	// note is below the staff
	       (note()->notePosition() >= note()->voice()->staff()->numberOfLines()*2)	// note is above the staff
	     )
	   ) {
	   	int direction = (note()->notePosition() > 0 ? 1 : -1);	// 1 falling, -1 rising
	   	int ledgerDist = qRound(9.0*s.z);	// distance between the ledger lines - notehead height
	   	
	   	// draw ledger lines in direction from the notehead to staff
		qreal ry = (direction==1)?_drawableContext->yPos():_drawableContext->yPos()+(_drawableContext->height()-1);
		ry *= s.z;
		QPen pen(s.color);
		pen.setWidthF(1.0*s.z);
		p->setPen(pen);
	   	for (int i=0;
	   	     i < ((note()->notePosition()*direction -
	   	          ((direction>0)?((note()->voice()->staff()->numberOfLines()-1)*2):0))/2);
	   	     ++i
	   	    )
		{
			ry -= ledgerDist*direction;
			p->drawLine(qRound(s.x - 4*s.z), qRound(ry), qRound(s.x + (_noteHeadWidth + 4)*s.z), qRound(ry));
		}
	}

	// Draw notehead
	s.y += _height*s.z/2;
	p->drawText(s.x, qRound(s.y), QString(_noteHeadCodepoint));

	if (note()->noteLength().musicLength() >= CAPlayableLength::Half) {
		// Draw stem and flag
		pen.setWidthF(_penWidth*s.z);	
		pen.setCapStyle(Qt::RoundCap);
		pen.setColor(s.color);
		p->setPen(pen);
		if (_stemDirection == CANote::StemUp) {
			s.x += qRound(_noteHeadWidth*s.z);
			p->drawLine(s.x, (int)(s.y-1*s.z), s.x, s.y-(int)(_stemLength*s.z));
			if(note()->noteLength().musicLength() >= CAPlayableLength::Eighth) {
				p->drawText((int)(s.x+0.6*s.z+0.5),(int)(s.y - _stemLength*s.z),QString(_stemUpCodepoint));
				s.x+=(int)(6*s.z+0.5);  // additional X-offset for dots
			}
		} else {
			s.x+=(int)(0.6*s.z+0.5);
			p->drawLine(s.x, (int)(s.y+1*s.z), s.x, s.y+(int)(_stemLength*s.z));
			if(note()->noteLength().musicLength() >= CAPlayableLength::Eighth)
				p->drawText((int)(s.x+0.4*s.z+0.5),(int)(s.y + (_stemLength+5)*s.z),QString(_stemDownCodepoint));
			s.x+=(int)(_noteHeadWidth*s.z+0.5);
		}
	}
	
	// Draw Dots
	float delta=4*s.z;
	for (int i=0; i<note()->playableLength().dotted(); i++) {
		pen.setWidth((int)(2.7*s.z+0.5) + 1);
		pen.setCapStyle(Qt::RoundCap);
		pen.setColor(s.color);
		p->setPen(pen);
		p->drawPoint((int)(s.x + delta + 0.5), (int)(s.y - 1.7*s.z + 0.5));
		delta += 3*s.z;
	}
		
	s.x += (int)(delta+0.5);
}

CADrawableNote *CADrawableNote::clone(CADrawableContext* newContext) {
	return new CADrawableNote(note(), (newContext)?newContext:_drawableContext, _xPos, _yPos + _height/2);
}

void CADrawableNote::setXPos(int xPos) {
	_xPos = xPos;
}

void CADrawableNote::setYPos(int yPos) {
	switch ( note()->playableLength().musicLength() ) {
	case CAPlayableLength::Quarter:
		_yPos = yPos - _height/2;
		break;
	case CAPlayableLength::Half:
		_yPos = yPos - _height/2;
		break;
	case CAPlayableLength::Whole:
		_yPos = yPos - _height/2;
		break;
	case CAPlayableLength::Breve:
		_yPos = yPos - _height/2;
		break;
	}
}
