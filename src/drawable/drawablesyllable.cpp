/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "drawable/drawablesyllable.h"
#include "drawable/drawablelyricscontext.h"

#include "core/lyricscontext.h"

#include <QPainter>

CADrawableSyllable::CADrawableSyllable( CASyllable* s, CADrawableLyricsContext* c, int x, int y )
 : CADrawableMusElement(s, c, x, y) {
	setDrawableMusElementType( DrawableSyllable );
	setWidth( syllable()->text().size() * 4 );
	setHeight( 15 );
}

CADrawableSyllable::~CADrawableSyllable() {
}

void CADrawableSyllable::draw(QPainter *p, const CADrawSettings s) {
	p->setPen(QPen(s.color));
	p->setFont( QFont("Century Schoolbook L", qRound(s.z*12)) );
	p->drawText( s.x, s.y+qRound(height()*s.z), syllable()->text().replace("_", " ") );
}

CADrawableSyllable *CADrawableSyllable::clone(CADrawableContext *c) {
	return new CADrawableSyllable(
		syllable(),
		(c?static_cast<CADrawableLyricsContext*>(c):static_cast<CADrawableLyricsContext*>(drawableContext())),
		xPos(),
		yPos()
	);
}
