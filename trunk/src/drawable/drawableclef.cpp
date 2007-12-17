/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <iostream>

#include <QFont>
#include <QPainter>

#include "drawable/drawableclef.h"
#include "drawable/drawablestaff.h"

#include "core/clef.h"

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
CADrawableClef::CADrawableClef(CAClef *musElement, CADrawableStaff *drawableStaff, int x, int y)
 : CADrawableMusElement(musElement, drawableStaff, x, y) {
	setDrawableMusElementType( CADrawableMusElement::DrawableClef );
	
	float lineSpace = drawableStaff->lineSpace();
	float bottom = drawableStaff->yPos()+drawableStaff->height();
	
	switch (clef()->clefType()) {
		case CAClef::G:
			_width = 21;
			_height = 68;
			_yPos = qRound( bottom - (((clef()->c1() + clef()->offset())/2.0) * lineSpace) - 0.89*_height );
			break;
		case CAClef::F:
			_width = 22;
			_height = 26;
			_yPos = qRound( bottom - (((clef()->c1() + clef()->offset())/2.0) * lineSpace) + 1.1*lineSpace );
			break;
		case CAClef::C:
			_width = 23;
			_height = 34;
			_yPos = qRound( bottom - (((clef()->c1() + clef()->offset())/2.0) * lineSpace) - 0.5*_height );
			break;
/*		case CAClef::PercussionHigh:
			_width = 23;
			_height = 44;
			break;
		case CAClef::PercussionLow:
			_width = 23;
			_height = 44;
			break;
		case CAClef::Tablature:
			_width = 23;
			_height = 44;
			break;*/
	}
	
	// make space for little 8 above/below, if needed
	if ( clef()->offset() )
		_height += CLEF_EIGHT_SIZE;
	
	if ( clef()->offset() > 0 )
		_yPos -= CLEF_EIGHT_SIZE;
	
	setNeededWidth( width() );
	setNeededHeight( height() );
}

void CADrawableClef::draw(QPainter *p, CADrawSettings s) {
	QFont font("Emmentaler");
	font.setPixelSize(qRound(35*s.z));
	p->setPen(QPen(s.color));
	p->setFont(font);
	
	/*
		Clef emmentaler glyphs: The first one is a normal clef (at the beginning of the system) and the other one is a smaller clef (at the center of the system)
		- 0xE191: C-clef
		- 0xE192: C-clef small
		- 0xE193: F-clef
		- 0xE194: F-clef small
		- 0xE195: G-clef
		- 0xE196: G-clef small
	*/
	switch (clef()->clefType()) {
		case CAClef::G:
			p->drawText(s.x, qRound(s.y + (clef()->offset()>0?CLEF_EIGHT_SIZE*s.z:0) + 0.63*(height() - (clef()->offset()?CLEF_EIGHT_SIZE:0))*s.z), QString(0xE195));
			break;
		case CAClef::F:
			p->drawText(s.x, qRound(s.y + (clef()->offset()>0?CLEF_EIGHT_SIZE*s.z:0) + 0.32*(height() - (clef()->offset()?CLEF_EIGHT_SIZE:0))*s.z), QString(0xE193));
			break;
		case CAClef::C:
			p->drawText(s.x, qRound(s.y + (clef()->offset()>0?CLEF_EIGHT_SIZE*s.z:0) + 0.5*(height() - (clef()->offset()?CLEF_EIGHT_SIZE:0))*s.z), QString(0xE191));
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
