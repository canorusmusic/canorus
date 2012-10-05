/*!
	Copyright (c) 2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "layout/drawablefiguredbasscontext.h"
#include "layout/drawablefiguredbassnumber.h"
#include <QBrush>
#include <QPainter>

CADrawableFiguredBassContext::CADrawableFiguredBassContext(CAFiguredBassContext *c, double x, double y)
 : CADrawableContext(c, x, y) {
	setDrawableContextType( DrawableFiguredBassContext );
	setWidth( 0 );
 	setHeight( 3*CADrawableFiguredBassNumber::DEFAULT_NUMBER_SIZE );
}

CADrawableFiguredBassContext::~CADrawableFiguredBassContext() {
}

CADrawableFiguredBassContext* CADrawableFiguredBassContext::clone() {
	return new CADrawableFiguredBassContext( figuredBassContext(), xPos(), yPos() );
}

void CADrawableFiguredBassContext::draw(QPainter *p, const CADrawSettings s) {
	p->fillRect(0, s.y, s.w, qRound(height()*s.z), QBrush(Qt::cyan));
}
