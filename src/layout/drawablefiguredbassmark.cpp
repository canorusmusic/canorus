/*!
	Copyright (c) 2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "layout/drawablefiguredbassmark.h"
#include "layout/drawablefiguredbasscontext.h"
#include "score/figuredbassmark.h"
#include <QPen>
#include <QPainter>

const double CADrawableFiguredBassMark::DEFAULT_NUMBER_SIZE = 14;

CADrawableFiguredBassMark::CADrawableFiguredBassMark( CAFiguredBassMark *f, CADrawableFiguredBassContext *context, double x, double y )
 : CADrawableMusElement(f, context, x, y) {
	setDrawableMusElementType( DrawableFiguredBassMark );
	int textWidth = 11;
	setWidth( textWidth < 11 ? 11 : textWidth ); // set minimum text width at least 11 points
	setHeight( qRound(DEFAULT_NUMBER_SIZE)*figuredBassMark()->numbers().size() );
}

CADrawableFiguredBassMark::~CADrawableFiguredBassMark() {
}

void CADrawableFiguredBassMark::draw(QPainter *p, const CADrawSettings s) {
	QPen pen(s.color);
	pen.setWidth( qRound(1.2*s.z) );
	pen.setCapStyle( Qt::RoundCap );
	p->setPen( pen );
	QFont font("Century Schoolbook L");
	font.setPixelSize( qRound(DEFAULT_NUMBER_SIZE*s.z) );
	p->setFont( font );

	for (int i=0; i<figuredBassMark()->numbers().size(); i++) {
		p->drawText( s.x, s.y+qRound(DEFAULT_NUMBER_SIZE*s.z*i), QString::number(figuredBassMark()->numbers()[i]) );
	}
}

CADrawableFiguredBassMark *CADrawableFiguredBassMark::clone(CADrawableContext *c) {
	return new CADrawableFiguredBassMark(
		figuredBassMark(),
		(c?static_cast<CADrawableFiguredBassContext*>(c):static_cast<CADrawableFiguredBassContext*>(drawableContext())),
		xPos(),
		yPos()
	);
}
