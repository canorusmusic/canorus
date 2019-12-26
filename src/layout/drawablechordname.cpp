/*!
	Copyright (c) 2019, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "layout/drawablechordname.h"
#include "layout/drawablechordnamecontext.h"

#include "score/chordnamecontext.h"

#include <QPainter>
#include <QFont>
#include <QFontMetrics>

const double CADrawableChordName::DEFAULT_TEXT_SIZE = 16;

CADrawableChordName::CADrawableChordName( CAChordName* s, CADrawableChordNameContext* c, double x, double y )
 : CADrawableMusElement(s, c, x, y) {
	setDrawableMusElementType( DrawableChordName );
	QFont font("Century Schoolbook L");
	font.setPixelSize( qRound(DEFAULT_TEXT_SIZE) );
	QFontMetrics fm(font);
	int textWidth = fm.width( drawableText() );
	setWidth( textWidth < 11 ? 11 : textWidth ); // set minimum text width at least 11 points
	setHeight( qRound(DEFAULT_TEXT_SIZE) );
}

CADrawableChordName::~CADrawableChordName() {
}

void CADrawableChordName::draw(QPainter *p, const CADrawSettings s) {
	QPen pen(s.color);
	pen.setWidth( qRound(1.2*s.z) );
	pen.setCapStyle( Qt::RoundCap );
	p->setPen( pen );
	QFont font("Century Schoolbook L");
	font.setPixelSize( qRound(DEFAULT_TEXT_SIZE*s.z) );
	p->setFont( font );
	p->drawText( s.x, s.y+qRound(height()*s.z), drawableText() );
}

CADrawableChordName *CADrawableChordName::clone(CADrawableContext *c) {
	return new CADrawableChordName(
		chordName(),
		(c?static_cast<CADrawableChordNameContext*>(c):static_cast<CADrawableChordNameContext*>(drawableContext())),
		xPos(),
		yPos()
	);
}

QString CADrawableChordName::drawableText() {
    // TODO: don't print quality modifier, if it's empty
    return CADiatonicPitch::diatonicPitchToString(chordName()->diatonicPitch()) + ":" + chordName()->qualityModifier();
}