/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "drawable/drawablesyllable.h"
#include "drawable/drawablelyricscontext.h"

#include "core/lyricscontext.h"

#include <QPainter>
#include <QFont>
#include <QFontMetrics>

const float CADrawableSyllable::DEFAULT_TEXT_SIZE = 16;

CADrawableSyllable::CADrawableSyllable( CASyllable* s, CADrawableLyricsContext* c, int x, int y )
 : CADrawableMusElement(s, c, x, y) {
	setDrawableMusElementType( DrawableSyllable );
	QFont font("Century Schoolbook L");
	font.setPixelSize( qRound(DEFAULT_TEXT_SIZE) );
	QFontMetrics fm(font);
	
	setWidth( fm.width( textToDrawableText(s->text()) ) );
	setHeight( qRound(DEFAULT_TEXT_SIZE) );
	
	setNeededWidth( width() );
	setNeededHeight( height() );
}

CADrawableSyllable::~CADrawableSyllable() {
}

void CADrawableSyllable::draw(QPainter *p, const CADrawSettings s) {
	p->setPen(QPen(s.color));
	QFont font("Century Schoolbook L");
	font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*s.z) );
	p->setFont( font );
	p->drawText( s.x, s.y+qRound(height()*s.z), textToDrawableText( syllable()->text() ) );
}

CADrawableSyllable *CADrawableSyllable::clone(CADrawableContext *c) {
	return new CADrawableSyllable(
		syllable(),
		(c?static_cast<CADrawableLyricsContext*>(c):static_cast<CADrawableLyricsContext*>(drawableContext())),
		xPos(),
		yPos()
	);
}
