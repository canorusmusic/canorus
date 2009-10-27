/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <iostream>

#include <QFont>
#include <QPainter>

#include "layout/drawableclef.h"
#include "layout/drawablestaff.h"

#include "score/clef.h"
#include "canorus.h"

const int CADrawableClef::CLEF_EIGHT_SIZE = 8;

/*!
	\class CADrawableClef
	\brief Drawable instance of CAClef
	This class draws the clef to the canvas.
*/

/*!
	Default constructor.

	\param clef Pointer to the logical CAClef.
	\param x X coordinate of the left-margin of the clef.
	\param y Y coordinate of the top of the staff. (WARNING! Not top of the clef!)
*/
CADrawableClef::CADrawableClef(CAClef *musElement, CADrawableStaff *drawableStaff, double x, double y)
 : CADrawableMusElement(musElement, drawableStaff, x, y) {
	setDrawableMusElementType( CADrawableMusElement::DrawableClef );

	double lineSpace = drawableStaff->lineSpace();
	double bottom = drawableStaff->yPos()+drawableStaff->height();

	switch (clef()->clefType()) {
		case CAClef::G:
			setWidth( 21 );
			setHeight( 68 );
			setYPos( bottom - (((clef()->c1() + clef()->offset())/2.0) * lineSpace) - 0.89*height() );
			break;
		case CAClef::F:
			setWidth( 22 );
			setHeight( 26 );
			setYPos( bottom - (((clef()->c1() + clef()->offset())/2.0) * lineSpace) + 1.1*lineSpace );
			break;
		case CAClef::C:
			setWidth( 23 );
			setHeight( 34 );
			setYPos( bottom - (((clef()->c1() + clef()->offset())/2.0) * lineSpace) - 0.5*height() );
			break;
		case CAClef::PercussionHigh:
		case CAClef::PercussionLow:
		case CAClef::Tab:  // TODO
			setWidth( 23 );
			setHeight( 34 );
			break;
	}

	// make space for little 8 above/below, if needed
	if ( clef()->offset() )
		setHeight( height()+CLEF_EIGHT_SIZE) ;

	if ( clef()->offset() > 0 )
		setYPos( yPos()-CLEF_EIGHT_SIZE );
}

void CADrawableClef::draw(QPainter *p, CADrawSettings s) {
	QFont font("Emmentaler");
	font.setPixelSize(qRound(35*s.z));
	p->setPen(QPen(s.color));
	p->setFont(font);

	/*
		There are two glyphs for each clef type: a normal clef (placed at the beginning of the system) and a smaller one (at the center of the system, key change).
		clefs.G, clefs.G_change, ...
	*/
	switch (clef()->clefType()) {
		case CAClef::G:
			p->drawText(s.x, qRound(s.y + (clef()->offset()>0?CLEF_EIGHT_SIZE*s.z:0) + 0.63*(height() - (clef()->offset()?CLEF_EIGHT_SIZE:0))*s.z), QString(CACanorus::fetaCodepoint("clefs.G")));
			break;
		case CAClef::F:
			p->drawText(s.x, qRound(s.y + (clef()->offset()>0?CLEF_EIGHT_SIZE*s.z:0) + 0.32*(height() - (clef()->offset()?CLEF_EIGHT_SIZE:0))*s.z), QString(CACanorus::fetaCodepoint("clefs.F")));
			break;
		case CAClef::C:
			p->drawText(s.x, qRound(s.y + (clef()->offset()>0?CLEF_EIGHT_SIZE*s.z:0) + 0.5*(height() - (clef()->offset()?CLEF_EIGHT_SIZE:0))*s.z), QString(CACanorus::fetaCodepoint("clefs.C")));
			break;
	}

	if (clef()->offset()) {
		QFont number("Century Schoolbook L");
		number.setPixelSize( qRound((CLEF_EIGHT_SIZE+5)*s.z) );
		number.setStyle( QFont::StyleItalic );
		p->setFont(number);

		if ( clef()->offset()>0 ) {
			p->drawText( qRound(s.x+(width()/2-4)*s.z), qRound(s.y + CLEF_EIGHT_SIZE*s.z), QString::number(clef()->offset()+1) );
		} else {
			p->drawText( qRound(s.x+(width()/2-4)*s.z), qRound(s.y + height()*s.z), QString::number(qAbs(clef()->offset()-1)) );
		}
	}
}

CADrawableClef* CADrawableClef::clone(CADrawableContext* newContext) {
	return (new CADrawableClef(clef(), (CADrawableStaff*)((newContext)?newContext:_drawableContext), xPos(), _drawableContext->yPos()));
}
