/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QFont>
#include <QPainter>

#include "drawable/drawablemark.h"
#include "drawable/drawablecontext.h"
#include "drawable/drawablenote.h" // needed for tempo mark

#include "interface/mididevice.h" // needed for instrument change

#include "core/note.h"            // needed for tempo mark
#include "core/mark.h"
#include "core/articulation.h"
#include "core/text.h"
#include "core/dynamic.h"
#include "core/instrumentchange.h"
#include "core/fermata.h"
#include "core/tempo.h"
#include "canorus.h"

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
	_tempoNote = 0;
	_tempoDNote = 0;
	
	if ( mark->markType()==CAMark::Text ) {
		QFont font("FreeSans");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE) );
		QFontMetrics fm(font);
		
		int textWidth = fm.width( static_cast<CAText*>(this->mark())->text() );
		setWidth( textWidth < 11 ? 11 : textWidth ); // set minimum text width at least 11 points
		setHeight( qRound(DEFAULT_TEXT_SIZE) );
	} else
	if ( mark->markType()==CAMark::Dynamic ) {
		QFont font("Emmentaler");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE) );
		QFontMetrics fm(font);
		
		int textWidth = fm.width( static_cast<CADynamic*>(this->mark())->text() );
		setWidth( textWidth < 11 ? 11 : textWidth ); // set minimum text width at least 11 points
		setHeight( qRound(DEFAULT_TEXT_SIZE) );
	} else
	if ( mark->markType()==CAMark::Fermata) {
		setWidth( 25 );
		setHeight( 20 );
	} else
	if ( mark->markType()==CAMark::InstrumentChange ) {
		QFont font("FreeSans");
		font.setStyle( QFont::StyleItalic );
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE) );
		QFontMetrics fm(font);
		
		int textWidth = fm.width( CACanorus::midiDevice()->GM_INSTRUMENTS[static_cast<CAInstrumentChange*>(this->mark())->instrument()] );
		setWidth( textWidth < 11 ? 11 : textWidth ); // set minimum text width at least 11 points
		setHeight( qRound(DEFAULT_TEXT_SIZE) );
	} else
	if ( mark->markType()==CAMark::Tempo ) {
		setWidth( 40 );
		setHeight( qRound(DEFAULT_TEXT_SIZE) );
		_tempoNote = new CANote( static_cast<CATempo*>(mark)->beat(), 0, 0, 0, 0, static_cast<CATempo*>(mark)->beatDotted() );
		_tempoDNote = new CADrawableNote( _tempoNote, dContext, x, y );
	} else {
		setWidth( 11 ); // set minimum text width at least 11 points
		setHeight( qRound(DEFAULT_TEXT_SIZE) );
	}
	
	setNeededWidth( width() );
	setNeededHeight( height() );
}

CADrawableMark::~CADrawableMark() {
	if ( _tempoDNote ) delete _tempoDNote;
	if ( _tempoNote ) delete _tempoNote;
}

void CADrawableMark::draw(QPainter *p, CADrawSettings s) {
	p->setPen(QPen(s.color));
		
	switch ( mark()->markType() ) {
	case CAMark::Dynamic: {
		QFont font("Emmentaler");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*s.z) );
		p->setFont(font);
		
		p->drawText( s.x, s.y, static_cast<CADynamic*>(mark())->text() );
		break;
	}
	case CAMark::Text: {
		QFont font("FreeSans");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*s.z) );
		p->setFont(font);
		
		p->drawText( s.x, s.y, static_cast<CAText*>(mark())->text() );
		break;
	}
	case CAMark::InstrumentChange: {
		QFont font("FreeSans");
		font.setStyle( QFont::StyleItalic );
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*s.z) );
		p->setFont(font);
		
		p->drawText( s.x, s.y, CACanorus::midiDevice()->GM_INSTRUMENTS[static_cast<CAInstrumentChange*>(this->mark())->instrument()] );
		break;
	}
	case CAMark::Fermata: {
		QFont font("Emmentaler");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*s.z) );
		p->setFont(font);
		
		switch ( static_cast<CAFermata*>(mark())->fermataType() ) {
			case CAFermata::NormalFermata: p->drawText( s.x, s.y, QString(0xE150) ); break;
			case CAFermata::ShortFermata: p->drawText( s.x, s.y, QString(0xE152) ); break;
			case CAFermata::LongFermata: p->drawText( s.x, s.y, QString(0xE154) ); break;
			case CAFermata::VeryLongFermata: p->drawText( s.x, s.y, QString(0xE156) ); break;
		}
		break;
	}
	case CAMark::Tempo: {
		_tempoDNote->draw( p, s );
		
		s.x += qRound(_tempoDNote->width()*s.z);
		QFont font("FreeSans");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*s.z) );
		p->setFont(font);
		p->drawText( s.x, s.y, QString(" = ") + QString::number( static_cast<CATempo*>(mark())->bpm() ) );
		break;
	}
	case CAMark::Articulation: {
		QFont font("Emmentaler");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*1.4*s.z) );
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
