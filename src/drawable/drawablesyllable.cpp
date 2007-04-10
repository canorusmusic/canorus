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
}

CADrawableSyllable::~CADrawableSyllable() {
}

void CADrawableSyllable::draw(QPainter *p, const CADrawSettings s) {
	p->setFont( QFont("Century Schoolbook L", qRound(s.z*14)) );
	p->drawText( s.x, s.y, syllable()->text() );
}

CADrawableSyllable *CADrawableSyllable::clone(CADrawableContext *c) {
	return new CADrawableSyllable(
		syllable(),
		(c?static_cast<CADrawableLyricsContext*>(c):static_cast<CADrawableLyricsContext*>(drawableContext())),
		xPos(),
		yPos()
	);
}
