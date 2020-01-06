/*!
	Copyright (c) 2019, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "layout/drawablechordnamecontext.h"
#include "score/chordnamecontext.h"
#include "layout/drawablechordname.h"

#include <QPainter>
#include <QBrush>

/*!
	Vertical space between the top edge of the chord name context and the top border of the chord name.
*/
const double CADrawableChordNameContext::DEFAULT_CHORDNAME_VERTICAL_SPACING = 2;

/*!
	Drawable instance of the chord name context.

	\sa CAChordNameContext
*/
CADrawableChordNameContext::CADrawableChordNameContext(CAChordNameContext *c, double x, double y)
 : CADrawableContext(c, x, y) {
	setDrawableContextType( DrawableChordNameContext );
	setWidth( 0 );
	setHeight( CADrawableChordName::DEFAULT_TEXT_SIZE + 2*DEFAULT_CHORDNAME_VERTICAL_SPACING );
}

CADrawableChordNameContext::~CADrawableChordNameContext() {
}

CADrawableChordNameContext* CADrawableChordNameContext::clone() {
	return new CADrawableChordNameContext( chordNameContext(), xPos(), yPos() );
}

void CADrawableChordNameContext::draw(QPainter *p, const CADrawSettings s) {
    QColor bColor = Qt::darkBlue;
    bColor.setAlphaF(0.2);
    p->fillRect(0, s.y, s.w, qRound(height() * s.z), QBrush(bColor));
}
