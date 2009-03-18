/*!
	Copyright (c) 2006-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "drawable/drawablerest.h"
#include "drawable/drawablecontext.h"
#include "drawable/drawablestaff.h"
#include "core/rest.h"
#include "canorus.h"

#include <QPainter>

CADrawableRest::CADrawableRest(CARest *rest, CADrawableContext *drawableContext, double x, double y)
 : CADrawableMusElement(rest, drawableContext, x, y) {
	_drawableMusElementType = CADrawableMusElement::DrawableRest;

	if (drawableContext->drawableContextType() != CADrawableContext::DrawableStaff)
		return;

	switch ( rest->playableLength().musicLength() ) {
	case CAPlayableLength::HundredTwentyEighth:
		setWidth( 16 );
		setHeight( 49 );
		break;

	case CAPlayableLength::SixtyFourth:
		setWidth( 14 );
		setHeight( 41 );
		break;

	case CAPlayableLength::ThirtySecond:
		setWidth( 12 );
		setHeight( 33 );
		setYPos(y + 2);
		break;

	case CAPlayableLength::Sixteenth:
		setWidth( 10 );
		setHeight( 24 );
		setYPos(y + static_cast<CADrawableStaff*>(drawableContext)->lineSpace());
		break;

	case CAPlayableLength::Eighth:
		setWidth( 8 );
		setHeight( 17 );
		setYPos(y + static_cast<CADrawableStaff*>(drawableContext)->lineSpace());
		break;

	case CAPlayableLength::Quarter:
		setWidth( 8 );
		setHeight( 20 );
		setYPos(y + static_cast<CADrawableStaff*>(drawableContext)->lineSpace());
		break;

	case CAPlayableLength::Half:
		setWidth( 12 );
		setHeight( 5 );
		setYPos(y + 1.5*static_cast<CADrawableStaff*>(drawableContext)->lineSpace());
		break;

	case CAPlayableLength::Whole:
		setWidth( 12 );
		setHeight( 5 );
		//values in constructor are the notehead center coords. yPos represents the top of the stem.
		setYPos(y + static_cast<CADrawableStaff*>(drawableContext)->lineSpace());
		break;

	case CAPlayableLength::Breve:
		setWidth( 4 );
		setHeight( 9 );
		setYPos(y + static_cast<CADrawableStaff*>(drawableContext)->lineSpace());
		break;
	}

	_restWidth = _width;

	if (rest->playableLength().dotted()) {
		setWidth( width() + 3 );
		for (int i=0; i<rest->playableLength().dotted(); i++)
			setWidth( width() + 2 );
	}
}

CADrawableRest::~CADrawableRest() {
}

CADrawableRest *CADrawableRest::clone(CADrawableContext* newContext) {
	return new CADrawableRest(rest(), (newContext)?newContext:_drawableContext, xPos(), yPos());
}

void CADrawableRest::draw(QPainter *p, CADrawSettings s) {
	QFont font("Emmentaler");
	font.setPixelSize(qRound(35*s.z));

	p->setPen(QPen(s.color));
	p->setFont(font);

	QPen pen;
	switch ( rest()->playableLength().musicLength() ) {
	case CAPlayableLength::HundredTwentyEighth: {
		p->drawText(qRound(s.x + 4*s.z), qRound(s.y + (2.6*((CADrawableStaff*)_drawableContext)->lineSpace())*s.z), QString(CACanorus::fetaCodepoint("rests.7")));
		break;
	}
	case CAPlayableLength::SixtyFourth: {
		p->drawText(qRound(s.x + 3*s.z), qRound(s.y + (1.75*((CADrawableStaff*)_drawableContext)->lineSpace())*s.z), QString(CACanorus::fetaCodepoint("rests.6")));
		break;
	}
	case CAPlayableLength::ThirtySecond: {
		p->drawText(qRound(s.x + 2.5*s.z), qRound(s.y + (1.8*((CADrawableStaff*)_drawableContext)->lineSpace())*s.z), QString(CACanorus::fetaCodepoint("rests.5")));
		break;
	}
	case CAPlayableLength::Sixteenth: {
		p->drawText(qRound(s.x + 1*s.z), qRound(s.y + (((CADrawableStaff*)_drawableContext)->lineSpace()-0.9)*s.z), QString(CACanorus::fetaCodepoint("rests.4")));
		break;
	}
	case CAPlayableLength::Eighth: {
		p->drawText(s.x, qRound(s.y + (((CADrawableStaff*)_drawableContext)->lineSpace()-0.9)*s.z), QString(CACanorus::fetaCodepoint("rests.3")));
		break;
	}
	case CAPlayableLength::Quarter: {
		p->drawText(s.x,qRound(s.y + 0.5*height()*s.z),QString(CACanorus::fetaCodepoint("rests.2")));
		break;
	}
	case CAPlayableLength::Half: {
		p->drawText(s.x,qRound(s.y + height()*s.z + 0.5), QString(CACanorus::fetaCodepoint("rests.1")));
		break;
	}
	case CAPlayableLength::Whole: {
		p->drawText(s.x, s.y, QString(CACanorus::fetaCodepoint("rests.0")));
		break;
	}
	case CAPlayableLength::Breve: {
		p->drawText(s.x, qRound(s.y + height()*s.z), QString(CACanorus::fetaCodepoint("rests.M1")));
		break;
	}
	}

	///////////////
	// Draw Dots //
	///////////////
	float delta=4*s.z;
	for (int i=0; i<rest()->playableLength().dotted(); i++) {
		pen.setWidth(qRound(2.7*s.z+0.5) + 1);
		pen.setCapStyle(Qt::RoundCap);
		pen.setColor(s.color);
		p->setPen(pen);
		p->drawPoint(qRound(s.x + _restWidth*s.z + delta), qRound(s.y + 0.3*_height*s.z));
		delta += 3*s.z;
	}

}
