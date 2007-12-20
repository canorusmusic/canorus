/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QFont>
#include <QPainter>

#include "drawable/drawablemark.h"
#include "drawable/drawablecontext.h"

#include "core/mark.h"
#include "core/articulation.h"
#include "core/text.h"

const int CADrawableMark::DEFAULT_TEXT_SIZE = 20;

/*!
	\class CADrawableMark
	\brief Drawable instance of marks
	This class draws the actual marks on the canvas.
*/

/*!
	Default constructor.
	
	\param mark Pointer to the model mark.
	\param x Left border of the associated element.
	\param y Bottom border of the mark.
*/
CADrawableMark::CADrawableMark( CAMark *mark, CADrawableContext *dContext, int x, int y)
 : CADrawableMusElement( mark, dContext, x, y ) {
	setDrawableMusElementType( CADrawableMusElement::DrawableMark );
	
	QFont font("FreeSans");
	font.setPixelSize( qRound(DEFAULT_TEXT_SIZE) );
	QFontMetrics fm(font);
	
	if ( mark->markType()==CAMark::Text ) {
		int textWidth = fm.width( static_cast<CAText*>(this->mark())->text() );
		setWidth( textWidth < 11 ? 11 : textWidth ); // set minimum text width at least 11 points
		setHeight( qRound(DEFAULT_TEXT_SIZE) );
	}
	
	setNeededWidth( width() );
	setNeededHeight( height() );
}

void CADrawableMark::draw(QPainter *p, CADrawSettings s) {
	p->setPen(QPen(s.color));
		
	switch ( mark()->markType() ) {
	case CAMark::Text: {
		QFont font("FreeSans");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*s.z) );
		p->setFont(font);
		
		p->drawText( s.x, s.y, static_cast<CAText*>(mark())->text() );
		break;
	}
	case CAMark::Articulation: {
		QFont font("Emmentaler");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*s.z) );
		p->setFont(font);
		
		switch ( static_cast<CAArticulation*>(mark())->articulationType() ) {
			case CAArticulation::Accent:        p->drawText( s.x, s.y, QString(0xE159) ); break;
			case CAArticulation::Marcato:       p->drawText( s.x, s.y, QString(0xE161) ); break;
			case CAArticulation::Staccatissimo: p->drawText( s.x, s.y, QString(0xE15C) ); break;
			case CAArticulation::Espressivo:    p->drawText( s.x, s.y, QString(0xE15A) ); break;
			case CAArticulation::Staccato:      p->drawText( s.x, s.y, QString(0xE15B) ); break;
			case CAArticulation::Tenuto:        p->drawText( s.x, s.y, QString(0xE15E) ); break;
			case CAArticulation::Portato:       p->drawText( s.x, s.y, QString(0xE15F) ); break;
			case CAArticulation::UpBow:         p->drawText( s.x, s.y, QString(0xE165) ); break;
			case CAArticulation::DownBow:       p->drawText( s.x, s.y, QString(0xE166) ); break;
			case CAArticulation::Flageolet:     p->drawText( s.x, s.y, QString(0xE16E) ); break;
			case CAArticulation::LHeel:         p->drawText( s.x, s.y, QString(0xE16A) ); break;
			case CAArticulation::RHeel:         p->drawText( s.x, s.y, QString(0xE16B) ); break;
			case CAArticulation::LToe:          p->drawText( s.x, s.y, QString(0xE16C) ); break;
			case CAArticulation::RToe:          p->drawText( s.x, s.y, QString(0xE16D) ); break;
			case CAArticulation::Open:          p->drawText( s.x, s.y, QString(0xE163) ); break;
			case CAArticulation::Stopped:       p->drawText( s.x, s.y, QString(0xE164) ); break;
			case CAArticulation::Turn:          p->drawText( s.x, s.y, QString(0xE167) ); break;
			case CAArticulation::ReverseTurn:   p->drawText( s.x, s.y, QString(0xE168) ); break;
			case CAArticulation::Trill:         p->drawText( s.x, s.y, QString(0xE169) ); break;
			case CAArticulation::Prall:         p->drawText( s.x, s.y, QString(0xE17B) ); break;
			case CAArticulation::Mordent:       p->drawText( s.x, s.y, QString(0xE17C) ); break;
			case CAArticulation::PrallPrall:    p->drawText( s.x, s.y, QString(0xE17D) ); break;
			case CAArticulation::PrallMordent:  p->drawText( s.x, s.y, QString(0xE17E) ); break;
			case CAArticulation::UpPrall:       p->drawText( s.x, s.y, QString(0xE17F) ); break;
			case CAArticulation::DownPrall:     p->drawText( s.x, s.y, QString(0xE182) ); break;
			case CAArticulation::UpMordent:     p->drawText( s.x, s.y, QString(0xE180) ); break;
			case CAArticulation::DownMordent:   p->drawText( s.x, s.y, QString(0xE183) ); break;
			case CAArticulation::PrallDown:     p->drawText( s.x, s.y, QString(0xE181) ); break;
			case CAArticulation::PrallUp:       p->drawText( s.x, s.y, QString(0xE184) ); break;
			case CAArticulation::LinePrall:     p->drawText( s.x, s.y, QString(0xE185) ); break;

		}
		
		break;
	}
	}
}

CADrawableMark* CADrawableMark::clone( CADrawableContext* newContext ) {
	return new CADrawableMark( mark(), newContext?newContext:drawableContext(), xPos(), yPos() );
}
