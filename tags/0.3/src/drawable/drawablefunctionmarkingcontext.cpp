/** @file drawable/drawablefunctionmarkingcontext.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QPainter>
#include <QBrush>

#include "drawable/drawablefunctionmarkingcontext.h"
#include "core/functionmarkingcontext.h"

CADrawableFunctionMarkingContext::CADrawableFunctionMarkingContext(CAFunctionMarkingContext *context, int x, int y, int numberOfLines)
 : CADrawableContext(context, x, y) {
 	_drawableContextType = CADrawableContext::DrawableFunctionMarkingContext;
 	
 	_numberOfLines = numberOfLines;
 	_currentLineIdx = 0;
 	_width = 0;
 	_height = 45*numberOfLines - 10*(numberOfLines-1);
}

CADrawableFunctionMarkingContext::~CADrawableFunctionMarkingContext() {
}

void CADrawableFunctionMarkingContext::draw(QPainter *p, const CADrawSettings s) {
	p->fillRect(0, s.y, s.w, qRound(height()*s.z), QBrush(Qt::yellow));
}

CADrawableFunctionMarkingContext *CADrawableFunctionMarkingContext::clone() {
	return new CADrawableFunctionMarkingContext((CAFunctionMarkingContext*)_context, xPos(), yPos());
}

int CADrawableFunctionMarkingContext::yPosLine(CAFunctionMarkingLine part) {
	int yPos = _yPos;
	for (int i=0; i<_currentLineIdx; i++) {
		yPos += 35;	//height of a single line
	}
	if (part==Middle)
		yPos += 15;
	else if (part==Lower)
		yPos += 30;
	
	return yPos;
}
