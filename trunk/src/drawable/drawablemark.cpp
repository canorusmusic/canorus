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
	QFont font("FreeSans");
	font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*s.z) );
	p->setPen(QPen(s.color));
	p->setFont(font);
		
	if ( mark()->markType()==CAMark::Text ) {
		p->drawText( s.x, s.y, static_cast<CAText*>(mark())->text() );
	}
}

CADrawableMark* CADrawableMark::clone( CADrawableContext* newContext ) {
	return new CADrawableMark( mark(), newContext?newContext:drawableContext(), xPos(), yPos() );
}
