/*!
	Copyright (c) 2006-2010, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <iostream>

#include <QFont>

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
CADrawableClef::CADrawableClef(CAClef *musElement, CADrawableStaff *drawableStaff)
 : CADrawableMusElement(musElement, drawableStaff, DrawableClef) {
	double lineSpace = drawableStaff->lineSpace();
/*	double bottom = drawableStaff->yPos()+drawableStaff->height();
*/
	//	There are two glyphs for each clef type: a normal clef (placed at the beginning of the system) and a smaller one (at the center of the system, key change).
	//	clefs.G, clefs.G_change, ...

	QGraphicsSimpleTextItem *item = new QGraphicsSimpleTextItem(this);
	QFont font("Emmentaler");
	font.setPixelSize( 35 );
	item->setFont( font );

	switch (clef()->clefType()) {
		case CAClef::G:
			item->setText(QString(CACanorus::fetaCodepoint("clefs.G")));
			item->setPos(0, drawableStaff->boundingRect().height() - (((clef()->c1() + clef()->offset())/2.0) * drawableStaff->lineSpace()) - 67);
			break;
		case CAClef::F:
			item->setText(QString(CACanorus::fetaCodepoint("clefs.F")));
			item->setPos(0, drawableStaff->boundingRect().height() - (((clef()->c1() + clef()->offset())/2.0) * drawableStaff->lineSpace()) - 27);
			break;
		case CAClef::C:
			item->setText(QString(CACanorus::fetaCodepoint("clefs.C")));
			item->setPos(0, drawableStaff->boundingRect().height() - (((clef()->c1() + clef()->offset())/2.0) * drawableStaff->lineSpace()) - 50); // TODO: y offset?
			break;
	}
	addToGroup( item );

	// add ^8va, _8va numbers
	if (clef()->offset()) {
		QGraphicsSimpleTextItem *numberItem = new QGraphicsSimpleTextItem(this);
		numberItem->setText( QString::number(clef()->offset()+1) );
		QFont numberFont( "Century Schoolbook L" );
		numberFont.setPixelSize( CLEF_EIGHT_SIZE+5 );
		numberFont.setStyle( QFont::StyleItalic );
		numberItem->setFont( numberFont );
		addToGroup( numberItem );

		if ( clef()->offset()>0 ) {
			numberItem->setPos( 2, item->pos().y()-numberItem->boundingRect().height() - 55 );
		} else {
			numberItem->setPos( 2, item->boundingRect().height() - 55 );
		}
	}
}

/*void CADrawableClef::draw(QPainter *p, CADrawSettings s) {
	QFont font("Emmentaler");
	font.setPixelSize(qRound(35*s.z));
	p->setPen(QPen(color()));
	p->setFont(font);

	switch (clef()->clefType()) {
		case CAClef::G:
			p->drawText(xPos(), qRound(yPos() + (clef()->offset()>0?CLEF_EIGHT_SIZE*s.z:0) + 0.63*(height() - (clef()->offset()?CLEF_EIGHT_SIZE:0))*1), QString(CACanorus::fetaCodepoint("clefs.G")));
			//p->drawText(s.x, qRound(s.y + (clef()->offset()>0?CLEF_EIGHT_SIZE*s.z:0) + 0.63*(height() - (clef()->offset()?CLEF_EIGHT_SIZE:0))*s.z), QString(CACanorus::fetaCodepoint("clefs.G")));
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
}*/
