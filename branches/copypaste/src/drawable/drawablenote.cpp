/*!
	Copyright (c) 2006, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPainter>
#include <iostream>
#include "drawable/drawablenote.h"
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
	
	switch (n->playableLength().musicLength()) {
	case CAPlayableLength::HundredTwentyEighth:
	case CAPlayableLength::SixtyFourth:
	case CAPlayableLength::ThirtySecond:
	case CAPlayableLength::Sixteenth:
	case CAPlayableLength::Eighth:
	case CAPlayableLength::Quarter:
	case CAPlayableLength::Half:
		_width = 11;
		_height = 10;
		_yPos = (int)(y - _height/2.0 + 0.5);
		_xPos = x;
		break;
		
	case CAPlayableLength::Whole:
		_width = 16;
		_height = 8;
		_yPos = (int)(y - _height/2.0 + 0.5);
		_xPos = x;
		break;
	
	case CAPlayableLength::Breve:
		_width = 21;
		_height = 8;
		_yPos = (int)(y - _height/2.0 + 0.5);
		_xPos = x;
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
	   	int y = (int)(s.y + ((height()*s.z)/2 + 0.5));	// initial y
	   	if ((note()->notePosition() % 2) != 0)	// if the note is not on the ledger line, shift the ledger line by half space
	   		y += (ledgerDist/2)*direction;

		QPen pen(s.color);
		pen.setWidth((int)(1.0*s.z));
		p->setPen(pen);
	   	for (int i=0;
	   	     i < ((note()->notePosition()*direction -
	   	          ((direction>0)?((note()->voice()->staff()->numberOfLines()-1)*2):0))/2);
	   	     ++i, y += (ledgerDist*direction)
	   	    )
			p->drawLine((int)(s.x - 4*s.z), (int)(y), (int)(s.x + (_noteHeadWidth + 4)*s.z), (int)(y));
	}
	
	// Draw Noteheads
	switch ( note()->noteLength().musicLength() ) {
	case CAPlayableLength::HundredTwentyEighth: {
		//draw notehead
		s.y += (int)((_height*s.z)/2 + 0.5);
		p->drawText(s.x,(int)(s.y - 0.1*s.z),QString(0xE125));
		
		// draw stem and flag
		/// \todo Emmentaler font doesn't have 128th, 64th flag is drawn instead! Need to somehow compose the 128th flag? -Matevz
		pen.setWidth((int)(1.2*s.z));
		pen.setCapStyle(Qt::RoundCap);
		pen.setColor(s.color);
		p->setPen(pen);
		if (_stemDirection == CANote::StemUp) {
			s.x+=(int)(_noteHeadWidth*s.z+0.5);
			p->drawLine(s.x, (int)(s.y-1*s.z), s.x, s.y-(int)(HUNDREDTWENTYEIGHTH_STEM_LENGTH*s.z));
			p->drawText((int)(s.x+0.6*s.z+0.5),(int)(s.y - HUNDREDTWENTYEIGHTH_STEM_LENGTH*s.z),QString(0xE18A));
			s.x+=(int)(6*s.z+0.5);	// additional X-offset for dots
		} else {
			s.x+=(int)(0.6*s.z+0.5);
			p->drawLine(s.x, (int)(s.y+1*s.z), s.x, s.y+(int)(HUNDREDTWENTYEIGHTH_STEM_LENGTH*s.z));
			p->drawText((int)(s.x+0.4*s.z+0.5),(int)(s.y + (HUNDREDTWENTYEIGHTH_STEM_LENGTH+5)*s.z),QString(0xE190));
			s.x+=(int)(_noteHeadWidth*s.z+0.5);
		}
			
		break;
	}
	case CAPlayableLength::SixtyFourth: {
		//draw notehead
		s.y += (int)((_height*s.z)/2 + 0.5);
		p->drawText(s.x,(int)(s.y - 0.1*s.z),QString(0xE125));
		
		//draw stem and flag
		pen.setWidth((int)(1.2*s.z));
		pen.setCapStyle(Qt::RoundCap);
		pen.setColor(s.color);
		p->setPen(pen);
		if (_stemDirection == CANote::StemUp) {
			s.x+=(int)(_noteHeadWidth*s.z+0.5);
			p->drawLine(s.x, (int)(s.y-1*s.z), s.x, s.y-(int)(HUNDREDTWENTYEIGHTH_STEM_LENGTH*s.z));
			p->drawText((int)(s.x+0.6*s.z+0.5),(int)(s.y - HUNDREDTWENTYEIGHTH_STEM_LENGTH*s.z),QString(0xE18A));
			s.x+=(int)(6*s.z+0.5);	// additional X-offset for dots
		} else {
			s.x+=(int)(0.6*s.z+0.5);
			p->drawLine(s.x, (int)(s.y+1*s.z), s.x, s.y+(int)(HUNDREDTWENTYEIGHTH_STEM_LENGTH*s.z));
			p->drawText((int)(s.x+0.4*s.z+0.5),(int)(s.y + (HUNDREDTWENTYEIGHTH_STEM_LENGTH+5)*s.z),QString(0xE190));
			s.x+=(int)(_noteHeadWidth*s.z+0.5);
		}
		
		break;
	}
	case CAPlayableLength::ThirtySecond: {
		//draw notehead
		s.y += (int)((_height*s.z)/2 + 0.5);
		p->drawText(s.x,(int)(s.y - 0.1*s.z),QString(0xE125));
		
		//draw stem and flag
		pen.setWidth((int)(1.2*s.z));
		pen.setCapStyle(Qt::RoundCap);
		pen.setColor(s.color);
		p->setPen(pen);
		if (_stemDirection == CANote::StemUp) {
			s.x+=(int)(_noteHeadWidth*s.z+0.5);
			p->drawLine(s.x, (int)(s.y-1*s.z), s.x, s.y-(int)(THIRTYSECOND_STEM_LENGTH*s.z));
			p->drawText((int)(s.x+0.6*s.z+0.5),(int)(s.y - THIRTYSECOND_STEM_LENGTH*s.z),QString(0xE189));
			s.x+=(int)(6*s.z+0.5);	// additional X-offset for dots
		} else {
			s.x+=(int)(0.6*s.z+0.5);
			p->drawLine(s.x, (int)(s.y+1*s.z), s.x, s.y+(int)(THIRTYSECOND_STEM_LENGTH*s.z));
			p->drawText((int)(s.x+0.4*s.z+0.5),(int)(s.y + (THIRTYSECOND_STEM_LENGTH+5)*s.z),QString(0xE18F));
			s.x+=(int)(_noteHeadWidth*s.z+0.5);
		}
		
		break;
	}
	case CAPlayableLength::Sixteenth: {
		//draw notehead
		s.y += (int)((_height*s.z)/2 + 0.5);
		p->drawText(s.x,(int)(s.y - 0.1*s.z),QString(0xE125));
		
		//draw stem and flag
		pen.setWidth((int)(1.2*s.z));
		pen.setCapStyle(Qt::RoundCap);
		pen.setColor(s.color);
		p->setPen(pen);
		if (_stemDirection == CANote::StemUp) {
			s.x+=(int)(_noteHeadWidth*s.z+0.5);
			p->drawLine(s.x, (int)(s.y-1*s.z), s.x, s.y-(int)(SIXTEENTH_STEM_LENGTH*s.z));
			p->drawText((int)(s.x+0.6*s.z+0.5),(int)(s.y - SIXTEENTH_STEM_LENGTH*s.z),QString(0xE188));
			s.x+=(int)(6*s.z+0.5);	// additional X-offset for dots
		} else {
			s.x+=(int)(0.6*s.z+0.5);
			p->drawLine(s.x, (int)(s.y+1*s.z), s.x, s.y+(int)(SIXTEENTH_STEM_LENGTH*s.z));
			p->drawText((int)(s.x+0.4*s.z+0.5),(int)(s.y + (SIXTEENTH_STEM_LENGTH+5)*s.z),QString(0xE18E));
			s.x+=(int)(_noteHeadWidth*s.z+0.5);
		}
		
		break;
	}
	case CAPlayableLength::Eighth: {
		//draw notehead
		s.y += (int)((_height*s.z)/2 + 0.5);
		p->drawText(s.x,(int)(s.y - 0.1*s.z),QString(0xE125));
		
		//draw stem and flag
		pen.setWidth((int)(1.2*s.z));
		pen.setCapStyle(Qt::RoundCap);
		pen.setColor(s.color);
		p->setPen(pen);
		if (_stemDirection == CANote::StemUp) {
			s.x+=(int)(_noteHeadWidth*s.z+0.5);
			p->drawLine(s.x, (int)(s.y-1*s.z), s.x, s.y-(int)(EIGHTH_STEM_LENGTH*s.z));
			p->drawText((int)(s.x+0.6*s.z+0.5),(int)(s.y - EIGHTH_STEM_LENGTH*s.z),QString(0xE187));
			s.x+=(int)(6*s.z+0.5);	// additional X-offset for dots
		} else {
			s.x+=(int)(0.6*s.z+0.5);
			p->drawLine(s.x, (int)(s.y+1*s.z), s.x, s.y+(int)(EIGHTH_STEM_LENGTH*s.z));
			p->drawText((int)(s.x+0.4*s.z+0.5),(int)(s.y + (EIGHTH_STEM_LENGTH+5)*s.z),QString(0xE18B));
			s.x+=(int)(_noteHeadWidth*s.z+0.5);
		}
		
		break;
	}
	case CAPlayableLength::Quarter: {
		//draw notehead
		s.y += (int)((_height*s.z)/2 + 0.5);
		p->drawText(s.x,(int)(s.y - 0.1*s.z),QString(0xE125));
		
		//draw stem
		pen.setWidth((int)(1.2*s.z));
		pen.setCapStyle(Qt::RoundCap);
		pen.setColor(s.color);
		p->setPen(pen);
		if (_stemDirection == CANote::StemUp) {
			s.x+=(int)(_noteHeadWidth*s.z+0.5);
			p->drawLine(s.x, (int)(s.y-1*s.z), s.x, s.y-(int)(QUARTER_STEM_LENGTH*s.z));
		} else {
			s.x+=(int)(0.6*s.z+0.5);
			p->drawLine(s.x, (int)(s.y+1*s.z), s.x, s.y+(int)(QUARTER_STEM_LENGTH*s.z));
			s.x+=(int)(_noteHeadWidth*s.z+0.5);
		}
		
		break;
	}
	case CAPlayableLength::Half: {
		//draw notehead
		s.y += (int)((_height*s.z)/2 + 0.5);
		p->drawText(s.x,(int)(s.y - 0.1*s.z),QString(0xE124));
		
		//draw stem
		pen.setWidth((int)(1.3*s.z));
		pen.setCapStyle(Qt::RoundCap);
		pen.setColor(s.color);
		p->setPen(pen);
		if (_stemDirection == CANote::StemUp) {
			s.x+=(int)(_noteHeadWidth*s.z+0.5);
			p->drawLine(s.x, (int)(s.y-1*s.z), s.x, s.y-(int)(HALF_STEM_LENGTH*s.z));
		} else {
			s.x+=(int)(0.6*s.z+0.5);
			p->drawLine(s.x, (int)(s.y+1*s.z), s.x, s.y+(int)(HALF_STEM_LENGTH*s.z));
			s.x+=(int)(_noteHeadWidth*s.z+0.5);
		}
		
		break;
	}
	case CAPlayableLength::Whole: {
		//draw notehead
		s.y += (int)((_height*s.z)/2 + 0.5);
		p->drawText(s.x, (int)(s.y), QString(0xE123));
		s.x+=(int)(_noteHeadWidth*s.z+0.5);
		
		break;
	}
	case CAPlayableLength::Breve: {
		//draw notehead
		s.y += (int)((_height*s.z)/2 + 0.5);
		p->drawText((int)(s.x + 2*s.z + 0.5), (int)(s.y), QString(0xE122));
		s.x+=(int)(_noteHeadWidth*s.z+0.5);
		
		break;
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
