/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QFont>
#include <QPainter>

#include "layout/drawablemark.h"
#include "layout/drawablecontext.h"
#include "layout/drawablenote.h" // needed for tempo mark

#include "interface/mididevice.h" // needed for instrument change

#include "score/note.h"            // needed for tempo mark
#include "score/mark.h"
#include "score/articulation.h"
#include "score/text.h"
#include "score/bookmark.h"
#include "score/dynamic.h"
#include "score/instrumentchange.h"
#include "score/fermata.h"
#include "score/tempo.h"
#include "score/ritardando.h"
#include "score/crescendo.h"
#include "score/repeatmark.h"
#include "canorus.h"

const double CADrawableMark::DEFAULT_TEXT_SIZE = 16;
const double CADrawableMark::DEFAULT_PIXMAP_SIZE = 25;

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
CADrawableMark::CADrawableMark( CAMark *mark, CADrawableContext *dContext, double x, double y)
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
	case CAMark::BookMark: {
		QFont font("FreeSans");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE) );
		QFontMetrics fm(font);

		int textWidth = fm.width( static_cast<CABookMark*>(this->mark())->text() );
		setWidth( DEFAULT_PIXMAP_SIZE + textWidth );
		setHeight( qRound(DEFAULT_TEXT_SIZE) );
		_pixmap = new QPixmap( "images:mark/bookmark.svg" );
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
	case CAMark::Pedal: {
		setWidth( mark->timeLength()/10 );
		setHeight( 20 );
		setHScalable(true);
		break;
	}
	case CAMark::Fermata: {
		setWidth( 20 );
		setHeight( 15 );
		break;
	}
	case CAMark::InstrumentChange: {
		QFont font("FreeSans");
		font.setStyle( QFont::StyleItalic );
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE) );
		QFontMetrics fm(font);

		_pixmap = new QPixmap( "images:mark/instrumentchange.svg" );
		int textWidth = fm.width( CACanorus::midiDevice()->GM_INSTRUMENTS[static_cast<CAInstrumentChange*>(this->mark())->instrument()] );
		setWidth( DEFAULT_PIXMAP_SIZE + textWidth ); // set minimum text width at least 11 points
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
		_tempoNote = new CANote( CADiatonicPitch(), static_cast<CATempo*>(mark)->beat(), 0, 0 );
		_tempoDNote = new CADrawableNote( _tempoNote, dContext, x, y );
		break;
	}
	case CAMark::Ritardando: {
		setWidth( mark->timeLength()/10 );
		setHeight( qRound(DEFAULT_TEXT_SIZE) );
		setHScalable(true);
		break;
	}
	case CAMark::Fingering: {
		setXPos( xPos()+6 );
		QFont font("Emmentaler");
		font.setPixelSize( 11 );
		QFontMetrics fm(font);

		QString text = fingerListToString( static_cast<CAFingering*>(mark)->fingerList() );
		setWidth( fm.width( text ) ); // set minimum text width at least 11 points
		setHeight( 11 );

		break;
	}
	case CAMark::RepeatMark: {
		if (static_cast<CARepeatMark*>(mark)->repeatMarkType()==CARepeatMark::Volta)
			setWidth( 50 );
		else if ( static_cast<CARepeatMark*>(mark)->repeatMarkType()==CARepeatMark::DalCoda ||
				  static_cast<CARepeatMark*>(mark)->repeatMarkType()==CARepeatMark::DalSegno ||
				  static_cast<CARepeatMark*>(mark)->repeatMarkType()==CARepeatMark::DalVarCoda )
			setWidth( 40 );
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
	case CAMark::BookMark: {
		QFont font("FreeSans");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*s.z) );
		p->setFont(font);

		p->drawPixmap( s.x, s.y, _pixmap->scaled(qRound(DEFAULT_PIXMAP_SIZE*s.z), qRound(DEFAULT_PIXMAP_SIZE*s.z) ) );
		p->drawText( s.x+qRound((DEFAULT_PIXMAP_SIZE+1)*s.z), s.y+qRound(height()*s.z), static_cast<CAText*>(mark())->text() );
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
		p->drawPixmap( s.x, s.y, _pixmap->scaled(qRound(DEFAULT_PIXMAP_SIZE*s.z), qRound(DEFAULT_PIXMAP_SIZE*s.z) ) );
		QFont font("FreeSans");
		font.setItalic( true );
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*s.z) );
		p->setFont(font);

		p->drawText( s.x + qRound((DEFAULT_PIXMAP_SIZE+1)*s.z), s.y+qRound(height()*s.z), CACanorus::midiDevice()->GM_INSTRUMENTS[static_cast<CAInstrumentChange*>(mark())->instrument()] );
		break;
	}
	case CAMark::Fermata: {
		QFont font("Emmentaler");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*1.1*s.z) );
		p->setFont(font);

		int inverted=0;
		if ( mark()->associatedElement()->musElementType()==CAMusElement::Note && static_cast<CANote*>(mark()->associatedElement())->actualSlurDirection()==CASlur::SlurDown )
			inverted=1;

		int x = qRound(s.x + (width()*s.z)*0.4);
		int y = qRound(s.y + (inverted?0:(height()*s.z)));
		switch ( static_cast<CAFermata*>(mark())->fermataType() ) {
			case CAFermata::NormalFermata: p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.ufermata")+inverted) ); break;
			case CAFermata::ShortFermata: p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.ushortfermata")+inverted) ); break;
			case CAFermata::LongFermata: p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.ulongfermata")+inverted) ); break;
			case CAFermata::VeryLongFermata: p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.uverylongfermata")+inverted) ); break;
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
	case CAMark::Ritardando: {
		QFont font("FreeSans");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*s.z) );
		font.setItalic( true );
		p->setFont(font);

		p->drawText( s.x, s.y+qRound(height()*s.z), static_cast<CARitardando*>(mark())->ritardandoType()==CARitardando::Ritardando?"rit.":"accel." );
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
			case CARepeatMark::DalSegno:   p->drawText( s.x, s.y, QString(CACanorus::fetaCodepoint("scripts.segno")) ); break;
			case CARepeatMark::Coda:
			case CARepeatMark::DalCoda:    p->drawText( s.x, s.y, QString(CACanorus::fetaCodepoint("scripts.coda")) ); break;
			case CARepeatMark::VarCoda:
			case CARepeatMark::DalVarCoda: p->drawText( s.x, s.y, QString(CACanorus::fetaCodepoint("scripts.varcoda")) ); break;
		}

		if (r->repeatMarkType()==CARepeatMark::Volta) {
			p->drawLine( s.x, qRound(s.y+height()*s.z), s.x, s.y );
			p->drawLine( s.x, s.y, qRound(s.x+width()*s.z), s.y );
			p->drawText( s.x + qRound(5*s.z), qRound(s.y+(height()-5)*s.z), QString::number(r->voltaNumber())+"." );
		}

		break;
	}
	case CAMark::Fingering: {
		QFont font("Emmentaler");
		CAFingering *f = static_cast<CAFingering*>(mark());
		font.setPixelSize( f->fingerList()[0]>5?qRound(DEFAULT_TEXT_SIZE*2*s.z):qRound(DEFAULT_TEXT_SIZE*1.3*s.z) );
		font.setItalic( static_cast<CAFingering*>(mark())->isOriginal() );
		p->setFont(font);
		QString text = fingerListToString( static_cast<CAFingering*>(mark())->fingerList() );
		p->drawText( s.x, s.y + qRound(height()*s.z), text );

		break;
	}
	case CAMark::Pedal: {
		QFont font("Emmentaler");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*1.6*s.z) );
		p->setFont(font);
		p->drawText( s.x, s.y+qRound(height()*s.z), QString(CACanorus::fetaCodepoint("pedal.Ped")) );
		p->drawText( s.x+qRound((width()-10)*s.z), s.y+qRound(height()*s.z), QString(CACanorus::fetaCodepoint("pedal.*")) );

		break;
	}
	case CAMark::Articulation: {
		QFont font("Emmentaler");
		font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*1.4*s.z) );
		p->setFont(font);

		int x = s.x + qRound((width()/2.0)*s.z);
		int y = s.y + qRound(height()*s.z);
		switch ( static_cast<CAArticulation*>(mark())->articulationType() ) {
			case CAArticulation::Accent:        p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.sforzato")) ); break;
			case CAArticulation::Marcato:       p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.umarcato")) ); break;
			case CAArticulation::Staccatissimo: p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.ustaccatissimo")) ); break;
			case CAArticulation::Espressivo:    p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.espr")) ); break;
			case CAArticulation::Staccato:      p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.staccato")) ); break;
			case CAArticulation::Tenuto:        p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.tenuto")) ); break;
			case CAArticulation::Portato:       p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.uportato")) ); break;
			case CAArticulation::UpBow:         p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.upbow")) ); break;
			case CAArticulation::DownBow:       p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.downbow")) ); break;
			case CAArticulation::Flageolet:     p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.flageolet")) ); break;
			case CAArticulation::Open:          p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.open")) ); break;
			case CAArticulation::Stopped:       p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.stopped")) ); break;
			case CAArticulation::Turn:          p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.turn")) ); break;
			case CAArticulation::ReverseTurn:   p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.reverseturn")) ); break;
			case CAArticulation::Trill:         p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.trill")) ); break;
			case CAArticulation::Prall:         p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.prall")) ); break;
			case CAArticulation::Mordent:       p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.mordent")) ); break;
			case CAArticulation::PrallPrall:    p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.prallprall")) ); break;
			case CAArticulation::PrallMordent:  p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.prallmordent")) ); break;
			case CAArticulation::UpPrall:       p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.upprall")) ); break;
			case CAArticulation::DownPrall:     p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.downprall")) ); break;
			case CAArticulation::UpMordent:     p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.upmordent")) ); break;
			case CAArticulation::DownMordent:   p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.downmordent")) ); break;
			case CAArticulation::PrallDown:     p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.pralldown")) ); break;
			case CAArticulation::PrallUp:       p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.prallup")) ); break;
			case CAArticulation::LinePrall:     p->drawText( x, y, QString(CACanorus::fetaCodepoint("scripts.lineprall")) ); break;

		}

		break;
	}
	}
}

CADrawableMark* CADrawableMark::clone( CADrawableContext* newContext ) {
	return new CADrawableMark( mark(), newContext?newContext:drawableContext(), xPos(), yPos() );
}

/*!
	Converts the list of fingers to Emmentaler string.
*/
QString CADrawableMark::fingerListToString( const QList<CAFingering::CAFingerNumber> list ) {
	QString text;
	for (int i=0; i<list.size(); i++) {
		if (list[i]>0 && list[i]<6)
			text += QString::number(list[i]);
		else if (list[i]==CAFingering::Thumb)
			text += QString(CACanorus::fetaCodepoint("scripts.thumb"));
		else if (list[i]==CAFingering::LHeel)
			text += QString(CACanorus::fetaCodepoint("scripts.upedalheel"));
		else if (list[i]==CAFingering::RHeel)
			text += QString(CACanorus::fetaCodepoint("scripts.dpedalheel"));
		else if (list[i]==CAFingering::LToe)
			text += QString(CACanorus::fetaCodepoint("scripts.upedaltoe"));
		else if (list[i]==CAFingering::RToe)
			text += QString(CACanorus::fetaCodepoint("scripts.dpedaltoe"));
	}

	return text;
}
