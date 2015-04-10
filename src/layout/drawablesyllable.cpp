/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "layout/drawablesyllable.h"
#include "layout/drawablelyricscontext.h"

#include "score/lyricscontext.h"

#include <QPainter>
#include <QFont>
#include <QFontMetrics>

const double CADrawableSyllable::DEFAULT_TEXT_SIZE = 16;
const double CADrawableSyllable::DEFAULT_DASH_LENGTH = 5;

CADrawableSyllable::CADrawableSyllable( CASyllable* s, CADrawableLyricsContext* c, double x, double y )
 : CADrawableMusElement(s, c, x, y) {
	setDrawableMusElementType( DrawableSyllable );
	QFont font("Century Schoolbook L");
	font.setPixelSize( qRound(DEFAULT_TEXT_SIZE) );
	QFontMetrics fm(font);
	int textWidth = fm.width( textToDrawableText(s->text()) );
	setWidth( textWidth < 11 ? 11 : textWidth ); // set minimum text width at least 11 points
	setHeight( qRound(DEFAULT_TEXT_SIZE) );
}

CADrawableSyllable::~CADrawableSyllable() {
}

void CADrawableSyllable::draw(QPainter *p, const CADrawSettings s) {
	QPen pen(s.color);
	pen.setWidth( qRound(1.2*s.z) );
	pen.setCapStyle( Qt::RoundCap );
	p->setPen( pen );
	QFont font("Century Schoolbook L");
	font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*s.z) );
	p->setFont( font );
	p->drawText( s.x, s.y+qRound(height()*s.z), textToDrawableText( syllable()->text() ) );

	int textWidth = QFontMetrics(font).width( textToDrawableText(syllable()->text()) );
	if ( syllable()->hyphenStart() && (width()*s.z - textWidth) > qRound(DEFAULT_DASH_LENGTH*s.z) ) {
		p->drawLine( qRound(s.x + width()*s.z*0.5 + 0.5*textWidth - 0.5*s.z*DEFAULT_DASH_LENGTH), s.y + qRound(height()*s.z*0.7),
		             qRound(s.x + width()*s.z*0.5 + 0.5*textWidth + 0.5*s.z*DEFAULT_DASH_LENGTH), s.y + qRound(height()*s.z*0.7));
	} else
	if ( syllable()->melismaStart() && (width()*s.z - textWidth) > qRound(DEFAULT_DASH_LENGTH*s.z) ) {
		p->drawLine( s.x + textWidth, s.y + qRound(height()*s.z),
		             qRound(s.x + width()*s.z), s.y + qRound(height()*s.z));
	}
}

CADrawableSyllable *CADrawableSyllable::clone(CADrawableContext *c) {
	return new CADrawableSyllable(
		syllable(),
		(c?static_cast<CADrawableLyricsContext*>(c):static_cast<CADrawableLyricsContext*>(drawableContext())),
		xPos(),
		yPos()
	);
}
