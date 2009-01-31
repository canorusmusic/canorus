/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "drawable/drawablelyricscontext.h"
#include "core/lyricscontext.h"
#include "drawable/drawablesyllable.h"

#include <QPainter>
#include <QBrush>

/*!
	Vertical space between the top edge of the lyrics context and the top border of the lyrcs text.
*/
const float CADrawableLyricsContext::DEFAULT_TEXT_VERTICAL_SPACING = 3;

/*!
	Drawable instance of the lyrics context.
	
	\sa CALyricsContext
*/
CADrawableLyricsContext::CADrawableLyricsContext(CALyricsContext *c, int x, int y)
 : CADrawableContext(c, x, y) {
	setDrawableContextType( DrawableLyricsContext );
	setWidth( 0 );
 	setHeight( qRound(CADrawableSyllable::DEFAULT_TEXT_SIZE + 2*DEFAULT_TEXT_VERTICAL_SPACING) );
}

CADrawableLyricsContext::~CADrawableLyricsContext() {
}

CADrawableLyricsContext* CADrawableLyricsContext::clone() {
	return new CADrawableLyricsContext( lyricsContext(), xPos(), yPos() );
}

void CADrawableLyricsContext::draw(QPainter *p, const CADrawSettings s) {
	p->fillRect(0, s.y, s.w, qRound(height()*s.z), QBrush(Qt::green));
}
