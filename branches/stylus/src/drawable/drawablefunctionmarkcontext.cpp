/*!
	Copyright (c) 2006, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPainter>
#include <QBrush>

#include "drawable/drawablefunctionmarkcontext.h"
#include "core/functionmarkcontext.h"

CADrawableFunctionMarkContext::CADrawableFunctionMarkContext(CAFunctionMarkContext *context, int x, int y, int numberOfLines)
 : CADrawableContext(context, x, y) {
 	_drawableContextType = CADrawableContext::DrawableFunctionMarkContext;
 	
 	_numberOfLines = numberOfLines;
 	_currentLineIdx = 0;
 	_width = 0;
 	_height = 45*numberOfLines - 10*(numberOfLines-1);
}

CADrawableFunctionMarkContext::~CADrawableFunctionMarkContext() {
}

void CADrawableFunctionMarkContext::draw(QPainter *p, const CADrawSettings s) {
	p->fillRect(0, s.y, s.w, qRound(height()*s.z), QBrush(Qt::yellow));
}

CADrawableFunctionMarkContext *CADrawableFunctionMarkContext::clone() {
	return new CADrawableFunctionMarkContext((CAFunctionMarkContext*)_context, xPos(), yPos());
}

int CADrawableFunctionMarkContext::yPosLine(CAFunctionMarkLine part) {
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
