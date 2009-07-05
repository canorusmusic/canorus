/*!
	Copyright (c) 2006, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPainter>
#include <QBrush>

#include "layout/drawablefunctionmarkcontext.h"
#include "score/functionmarkcontext.h"

CADrawableFunctionMarkContext::CADrawableFunctionMarkContext(CAFunctionMarkContext *context, double x, double y, int numberOfLines)
 : CADrawableContext(context, x, y) {
 	setDrawableContextType( CADrawableContext::DrawableFunctionMarkContext );

 	_numberOfLines = numberOfLines;
 	_currentLineIdx = 0;
 	setWidth( 0 );
 	setHeight( 45*numberOfLines - 10*(numberOfLines-1) );
}

CADrawableFunctionMarkContext::~CADrawableFunctionMarkContext() {
}

void CADrawableFunctionMarkContext::draw(QPainter *p, const CADrawSettings s) {
	p->fillRect(0, s.y, s.w, qRound(height()*s.z), QBrush(Qt::yellow));
}

CADrawableFunctionMarkContext *CADrawableFunctionMarkContext::clone() {
	return new CADrawableFunctionMarkContext((CAFunctionMarkContext*)_context, xPos(), yPos());
}

double CADrawableFunctionMarkContext::yPosLine(CAFunctionMarkLine part) {
	double y = yPos();
	for (int i=0; i<_currentLineIdx; i++) {
		y += 35;	//height of a single line
	}
	if (part==Middle)
		y += 15;
	else if (part==Lower)
		y += 30;

	return y;
}
