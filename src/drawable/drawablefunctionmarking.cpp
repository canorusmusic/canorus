/** @file drawable/drawablefunctionmarking.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QPainter>
#include <QFont>

#include "drawable/drawablefunctionmarking.h"
#include "drawable/drawablefunctionmarkingcontext.h"
#include "core/functionmarking.h"

////////////////////////////////////////////////////
// class CADrawableFunctionMarking
////////////////////////////////////////////////////
CADrawableFunctionMarking::CADrawableFunctionMarking(CAFunctionMarking *function, CADrawableFunctionMarkingContext *context, int x, int y)
 : CADrawableMusElement(function, context, x, y) {
 	_drawableMusElementType = CADrawableMusElement::DrawableFunctionMarking;
 	
 	_extenderLineVisible = false;
 	_width=11;
 	_height=15;
 	_fontWidth = 10;
 	_neededWidth = _width;
 	_neededHeight = _height;
}

CADrawableFunctionMarking::~CADrawableFunctionMarking() {
	
}

void CADrawableFunctionMarking::draw(QPainter *p, CADrawSettings s) {
	QFont font("FreeSans");
	font.setPixelSize((int)(19*s.z));
	p->setPen(QPen(s.color));
	p->setFont(font);
	
	QString text;
	switch (functionMarking()->function()) {
		case CAFunctionMarking::I: text="I"; break;
		case CAFunctionMarking::II: text="II"; break;
		case CAFunctionMarking::III: text="III"; break;
		case CAFunctionMarking::IV: text="IV"; break;
		case CAFunctionMarking::V: text="V"; break;
		case CAFunctionMarking::VI: text="VI"; break;
		case CAFunctionMarking::VII: text="VII"; break;
		case CAFunctionMarking::T: text="T"; break;
		case CAFunctionMarking::S: text="S"; break;
		case CAFunctionMarking::D: text="D"; break;
		case CAFunctionMarking::F: text="F"; break;
		case CAFunctionMarking::N: text="N"; break;
		case CAFunctionMarking::L: text="L"; break;
	}
	
	p->drawText(s.x, s.y+(int)(_height*s.z+0.5), text);
	
	if (_extenderLineVisible)
		p->drawLine(s.x + (int)(_fontWidth*s.z+0.5), s.y+(int)((_height/2.0)*s.z+0.5),
		            s.x + (int)(_width*s.z+0.5), s.y+(int)((_height/2.0)*s.z+0.5));
}

CADrawableFunctionMarking *CADrawableFunctionMarking::clone() {
	return new CADrawableFunctionMarking(functionMarking(), drawableFunctionMarkingContext(), xPos(), yPos());
}

////////////////////////////////////////////////////
// class CADrawableFunctionMarkingSupport
////////////////////////////////////////////////////
CADrawableFunctionMarkingSupport::CADrawableFunctionMarkingSupport(CADrawableFunctionMarkingSupportType type, const QString key, CADrawableContext *c, int x, int y)
 : CADrawableMusElement(0, c, x, y) {
	_drawableMusElementType = CADrawableMusElement::DrawableFunctionMarkingSupport;
	_drawableFunctionMarkingSupportType = type;
	_key = key;
	_width = key.size()*7 + 7;
	_height = 14;
	_neededWidth = _width;
	_neededHeight = _height;
	_selectable = false;
}

CADrawableFunctionMarkingSupport::~CADrawableFunctionMarkingSupport() {
}
		
void CADrawableFunctionMarkingSupport::draw(QPainter *p, const CADrawSettings s) {
	QFont font("FreeSans");
	font.setPixelSize((int)(17*s.z));
	p->setPen(QPen(s.color));
	p->setFont(font);
	p->drawText(s.x, s.y+(int)(_height*s.z+0.5), _key+":");
}

CADrawableFunctionMarkingSupport *CADrawableFunctionMarkingSupport::clone() {
	switch (_drawableFunctionMarkingSupportType) {
		case Key:
			return new CADrawableFunctionMarkingSupport(Key, _key, _drawableContext, _xPos, _yPos);
			break;
	}
}
