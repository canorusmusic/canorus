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
#include "core/crescendo.h"
#include "core/repeatmark.h"
#include "canorus.h"

const int CADrawableMark::DEFAULT_TEXT_SIZE = 16;

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
	_pixmap = 0;
	
	switch (mark->markType()) {
	case CAMark::Text: {
		QFont font("FreeSans");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE) );
		QFontMetrics fm(font);
		
		int textWidth = fm.width( static_cast<CAText*>(this->mark())->text() );
		setWidth( textWidth < 11 ? 11 : textWidth ); // set minimum text width at least 11 points
		setHeight( qRound(DEFAULT_TEXT_SIZE) );
		break;
	}
	case CAMark::Dynamic: {
		QFont font("Emmentaler");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE) );
		QFontMetrics fm(font);
		
		int textWidth = fm.width( static_cast<CADynamic*>(this->mark())->text() );
		setWidth( textWidth < 11 ? 11 : textWidth ); // set minimum text width at least 11 points
		setHeight( qRound(DEFAULT_TEXT_SIZE) );
		break;
	}
	case CAMark::Crescendo: {
		setWidth( mark->timeLength()/10 );
		setHeight( static_cast<CACrescendo*>(mark)->finalVolume()/10 );
		setHScalable(true);
		break;
	}
	case CAMark::Fermata: {
		setWidth( 25 );
		setHeight( 20 );
		break;
	}
	case CAMark::InstrumentChange: {
		QFont font("FreeSans");
		font.setStyle( QFont::StyleItalic );
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE) );
		QFontMetrics fm(font);
		
		if ( CACanorus::locateResource("images/mark/instrumentchange.svg").size() )
			_pixmap = new QPixmap( CACanorus::locateResource("images/mark/instrumentchange.svg")[0] );
		int textWidth = fm.width( CACanorus::midiDevice()->GM_INSTRUMENTS[static_cast<CAInstrumentChange*>(this->mark())->instrument()] );
		setWidth( textWidth < 11 ? 11 : textWidth ); // set minimum text width at least 11 points
		setHeight( qRound(DEFAULT_TEXT_SIZE) );
		break;
	}
	case CAMark::RehersalMark: {
		setWidth( 11 );
		setHeight( qRound(DEFAULT_TEXT_SIZE) );
		break;
	}
	case CAMark::Tempo: {
		setWidth( 40 );
		setHeight( qRound(DEFAULT_TEXT_SIZE) );
		_tempoNote = new CANote( static_cast<CATempo*>(mark)->beat(), 0, 0, 0, 0, static_cast<CATempo*>(mark)->beatDotted() );
		_tempoDNote = new CADrawableNote( _tempoNote, dContext, x, y );
		break;
	}
	case CAMark::RepeatMark: {
		if (static_cast<CARepeatMark*>(mark)->repeatMarkType()==CARepeatMark::Volta)
			setWidth( 50 );
		else if ( static_cast<CARepeatMark*>(mark)->repeatMarkType()==CARepeatMark::DalCoda ||
				  static_cast<CARepeatMark*>(mark)->repeatMarkType()==CARepeatMark::DalSegno ||
				  static_cast<CARepeatMark*>(mark)->repeatMarkType()==CARepeatMark::DalVarCoda )
			setWidth( 50 );
		else
			setWidth( 25 );
		setHeight( qRound(DEFAULT_TEXT_SIZE) );
		break;
	}
	default: {
		setWidth( 11 ); // set minimum text width at least 11 points
		setHeight( qRound(DEFAULT_TEXT_SIZE) );
		break;
	}
	}
	
	setNeededWidth( width() );
	setNeededHeight( height() );
}

CADrawableMark::~CADrawableMark() {
	if ( _tempoDNote ) delete _tempoDNote;
	if ( _tempoNote ) delete _tempoNote;
	if ( _pixmap ) delete _pixmap;
}

void CADrawableMark::draw(QPainter *p, CADrawSettings s) {
	p->setPen(QPen(s.color));
	
	switch ( mark()->markType() ) {
	case CAMark::Dynamic: {
		QFont font("Emmentaler");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*s.z) );
		p->setFont(font);
		
		p->drawText( s.x, s.y+qRound(height()*s.z), static_cast<CADynamic*>(mark())->text() );
		break;
	}
	case CAMark::Crescendo: {
		if (static_cast<CACrescendo*>(mark())->crescendoType()==CACrescendo::Crescendo) {
			p->drawLine( s.x, qRound(s.y+(height()*s.z) / 2), qRound(s.x + width()*s.z), s.y );
			p->drawLine( s.x, qRound(s.y+(height()*s.z) / 2), qRound(s.x + width()*s.z), qRound(s.y+(height()*s.z)) );
		} else {
			p->drawLine( s.x, s.y, qRound(s.x + width()*s.z), qRound(s.y+(height()*s.z) / 2) );
			p->drawLine( s.x, qRound(s.y+(height()*s.z)), qRound(s.x + width()*s.z), qRound(s.y+(height()*s.z) / 2) );
		}
		break;
	}
	case CAMark::Text: {
		QFont font("FreeSans");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*s.z) );
		p->setFont(font);
		
		p->drawText( s.x, s.y+qRound(height()*s.z), static_cast<CAText*>(mark())->text() );
		break;
	}
	case CAMark::RehersalMark: {
		QFont font("FreeSans");
		font.setBold( true );
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*s.z) );
		p->setFont(font);
		
		p->drawRect( s.x, s.y, qRound(width()*s.z), qRound(height()*s.z) );
		p->drawText( s.x, s.y+qRound(height()*s.z), QString((char)('A'+rehersalMarkNumber())) );
		break;
	}
	case CAMark::InstrumentChange: {
		p->drawPixmap( s.x, s.y, _pixmap->scaled(qRound(25*s.z), qRound(25*s.z) ) );
		QFont font("FreeSans");
		font.setItalic( true );
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*s.z) );
		p->setFont(font);
		
		p->drawText( s.x, s.y+qRound(height()*s.z), CACanorus::midiDevice()->GM_INSTRUMENTS[static_cast<CAInstrumentChange*>(this->mark())->instrument()] );
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
		p->drawText( s.x, s.y+qRound(height()*s.z), QString(" = ") + QString::number( static_cast<CATempo*>(mark())->bpm() ) );
		break;
	}
	case CAMark::RepeatMark: {
		CARepeatMark *r = static_cast<CARepeatMark*>(mark());
		
		// draw "dal" if needed
		if ( r->repeatMarkType()==CARepeatMark::DalSegno ||
		     r->repeatMarkType()==CARepeatMark::DalCoda ||
		     r->repeatMarkType()==CARepeatMark::DalVarCoda ) {
			QFont font("Century Schoolbook L");
			font.setStyle( QFont::StyleItalic );
			font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*s.z) );
			p->setFont(font);
			p->drawText( s.x, s.y, QString("Dal"));
			s.x += qRound(45*s.z);
		}
		
		// draw the actual sign
		QFont font("Emmentaler");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*1.4*s.z) );
		p->setFont(font);
		switch ( static_cast<CARepeatMark*>(mark())->repeatMarkType() ) {
			case CARepeatMark::Segno:
			case CARepeatMark::DalSegno:   p->drawText( s.x, s.y, QString(0xE16F) ); break;
			case CARepeatMark::Coda:
			case CARepeatMark::DalCoda:    p->drawText( s.x, s.y, QString(0xE170) ); break;
			case CARepeatMark::VarCoda:
			case CARepeatMark::DalVarCoda: p->drawText( s.x, s.y, QString(0xE171) ); break;
		}
		
		if (r->repeatMarkType()==CARepeatMark::Volta) {
			p->drawLine( s.x, qRound(s.y+height()*s.z), s.x, s.y );
			p->drawLine( s.x, s.y, qRound(s.x+width()*s.z), s.y );
			p->drawText( s.x + qRound(5*s.z), qRound(s.y+(height()-5)*s.z), QString::number(r->voltaNumber())+"." );
		}
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
