/*!
	Copyright (c) 2006-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPainter>
#include <iostream>
#include "layout/drawablestaff.h"
#include "layout/drawablenote.h"
#include "layout/drawablecontext.h"
#include "layout/drawableaccidental.h"
#include "score/voice.h"
#include "score/staff.h"
#include "canorus.h"

const double CADrawableNote::HUNDREDTWENTYEIGHTH_STEM_LENGTH = 50;
const double CADrawableNote::SIXTYFOURTH_STEM_LENGTH = 43;
const double CADrawableNote::THIRTYSECOND_STEM_LENGTH = 37;
const double CADrawableNote::SIXTEENTH_STEM_LENGTH = 31;
const double CADrawableNote::EIGHTH_STEM_LENGTH = 31;
const double CADrawableNote::QUARTER_STEM_LENGTH = 31;
const double CADrawableNote::HALF_STEM_LENGTH = 31;
const double CADrawableNote::QUARTER_YPOS_DELTA = 21;
const double CADrawableNote::HALF_YPOS_DELTA = 23;

/*!
	Default constructor.

	\param x coordinate represents the left border of the notehead.
	\param y coordinate represents the center of the notehead.
*/
CADrawableNote::CADrawableNote(CANote *n, CADrawableContext *drawableContext, double x, double y, bool shadowNote, CADrawableAccidental *drawableAcc)
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
		_noteHeadGlyphName = "noteheads.s2";
		_penWidth = 1.2;
		setWidth( 11 );
		setHeight( 10 );
		break;

	case CAPlayableLength::Half:
		_noteHeadGlyphName = "noteheads.s1";
		_penWidth = 1.3;
		setWidth( 12 );
		setHeight( 10 );
		break;

	case CAPlayableLength::Whole:
		_noteHeadGlyphName = "noteheads.s0";
		_penWidth = 0;
		setWidth( 17 );
		setHeight( 8 );
		break;

	case CAPlayableLength::Breve:
		_noteHeadGlyphName = "noteheads.sM1";
		_penWidth = 0;
		setWidth( 18 );
		setHeight( 8 );
		break;
	case CAPlayableLength::Undefined:
		fprintf(stderr,"Warning: CADrawableNote::CADrawableNote - Unhandled Length %d\n",n->playableLength().musicLength());
		break;
	}
	setYPos( y - height()/2.0 );
	setXPos( x );

	int mlength = n->playableLength().musicLength();
	switch (mlength) {
	case CAPlayableLength::HundredTwentyEighth:
		/// \todo Emmentaler font doesn't have 128th, 64th flag is drawn instead! Need to somehow compose the 128th flag? -Matevz
		_stemLength = HUNDREDTWENTYEIGHTH_STEM_LENGTH;
		_flagUpGlyphName = "flags.u7";
		_flagDownGlyphName = "flags.d7";
		break;
	case CAPlayableLength::SixtyFourth:
		_stemLength = SIXTYFOURTH_STEM_LENGTH;
		_flagUpGlyphName = "flags.u6";
		_flagDownGlyphName = "flags.d6";
		break;
	case CAPlayableLength::ThirtySecond:
		_stemLength = THIRTYSECOND_STEM_LENGTH;
		_flagUpGlyphName = "flags.u5";
		_flagDownGlyphName = "flags.d5";
		break;
	case CAPlayableLength::Sixteenth:
		_stemLength = SIXTEENTH_STEM_LENGTH;
		_flagUpGlyphName = "flags.u4";
		_flagDownGlyphName = "flags.d4";
		break;
	case CAPlayableLength::Eighth:
		_stemLength = EIGHTH_STEM_LENGTH;
		_flagUpGlyphName = "flags.u3";
		_flagDownGlyphName = "flags.d3";
		break;
	case CAPlayableLength::Quarter:
		_stemLength = QUARTER_STEM_LENGTH;
		break;
	case CAPlayableLength::Half:
		_stemLength = HALF_STEM_LENGTH;
		break;
	case CAPlayableLength::Whole:
	case CAPlayableLength::Breve:
	case CAPlayableLength::Undefined:
		if(mlength < 0 /*|| mlength > HundredTwentyEighth*/) // Currently no refined check done
			fprintf(stderr,"Warning: CADrawableNote::CADrawableNote - Unhandled length %d",mlength);
		break;

	}

	_noteHeadWidth = width();

	if (n->playableLength().dotted()) {
		setWidth( width()+3 );
		for (int i=0; i<n->playableLength().dotted(); i++)
			setWidth( width()+2 );
	}

	_shadowNote = shadowNote;

	_drawLedgerLines = true;
}

CADrawableNote::~CADrawableNote() {
}

void CADrawableNote::draw(QPainter *p, CADrawSettings s) {
	QFont font("Emmentaler");
	font.setPixelSize(qRound(35*s.z));

	p->setPen(QPen(s.color));
	p->setFont(font);

	QPen pen;

	// Draw ledger lines
	if ( _drawLedgerLines && drawableContext() && drawableContext()->drawableContextType()==CADrawableContext::DrawableStaff &&
	     note() && note()->voice() && note()->voice()->staff() &&
	     ( (note()->notePosition() <= -2) ||	// note is below the staff
	       (note()->notePosition() >= note()->voice()->staff()->numberOfLines()*2)	// note is above the staff
	     )
	   ) {
	   	int direction = (note()->notePosition() > 0 ? 1 : -1);	// 1 falling, -1 rising
	   	double ledgerDist = static_cast<CADrawableStaff*>(drawableContext())->lineSpace()*s.z;	// distance between the ledger lines - notehead height

	   	// draw ledger lines in direction from the notehead to staff
		qreal ry = (direction==1)?_drawableContext->yPos():_drawableContext->yPos()+(_drawableContext->height());
		ry -= s.worldY;
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
	s.y += height()*s.z/2;
	p->drawText(s.x, s.y, QString(CACanorus::fetaCodepoint(_noteHeadGlyphName)));

	if (note()->noteLength().musicLength() >= CAPlayableLength::Half) {
		// Draw stem and flag
		pen.setWidthF(_penWidth*s.z);
		pen.setCapStyle(Qt::RoundCap);
		pen.setColor(s.color);
		p->setPen(pen);
		if (_stemDirection == CANote::StemUp) {
			s.x+=qRound(_noteHeadWidth*s.z); // increase X-offset before drawing the stem
			p->drawLine(s.x, qRound(s.y-1*s.z), s.x, s.y-qRound(_stemLength*s.z));
			if(note()->noteLength().musicLength() >= CAPlayableLength::Eighth) {
				p->drawText(qRound(s.x+0.6*s.z),qRound(s.y - _stemLength*s.z),QString(CACanorus::fetaCodepoint(_flagUpGlyphName)));
				s.x+=qRound(6*s.z); // additional X-offset for dots because of the flag on the right
			}
		} else {
			s.x+=qRound(0.6*s.z);
			p->drawLine(s.x, qRound(s.y+1*s.z), s.x, s.y+qRound(_stemLength*s.z));
			if(note()->noteLength().musicLength() >= CAPlayableLength::Eighth) {
				p->drawText(qRound(s.x+0.4*s.z),qRound(s.y + (_stemLength+5)*s.z),QString(CACanorus::fetaCodepoint(_flagDownGlyphName)));
			}
			s.x+=qRound(_noteHeadWidth*s.z); // increase X-offset after drawing the stem
		}
	} else {
		s.x+=qRound(_noteHeadWidth*s.z); // increase X-offset for drawing the dots
	}

	// Draw Dots
	float delta=4*s.z;
	for (int i=0; i<note()->playableLength().dotted(); i++) {
		pen.setWidth(qRound(2.7*s.z) + 1);
		pen.setCapStyle(Qt::RoundCap);
		pen.setColor(s.color);
		p->setPen(pen);
		p->drawPoint(qRound(s.x + delta), qRound(s.y - 1.7*s.z));
		delta += 4*s.z;
	}

	s.x += qRound(delta);
}

CADrawableNote *CADrawableNote::clone(CADrawableContext* newContext) {
	return new CADrawableNote(note(), (newContext)?newContext:_drawableContext, xPos(), yPos() + height()/2);
}
