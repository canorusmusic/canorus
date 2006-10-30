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

CADrawableFunctionMarkingContext::CADrawableFunctionMarkingContext(CAFunctionMarkingContext *context, int x, int y)
 : CADrawableContext(context, x, y) {
}

CADrawableFunctionMarkingContext::~CADrawableFunctionMarkingContext() {
}

void CADrawableFunctionMarkingContext::draw(QPainter *p, const CADrawSettings s) {
	p->fillRect(s.x, s.y, s.w, s.h, QBrush(Qt::yellow));
}

CADrawableFunctionMarkingContext *CADrawableFunctionMarkingContext::clone() {
	return new CADrawableFunctionMarkingContext((CAFunctionMarkingContext*)_context, xPos(), yPos());
}
