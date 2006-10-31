/** @file core/functionmarkingcontext.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "core/functionmarkingcontext.h"
#include "core/functionmarking.h"

CAFunctionMarkingContext::CAFunctionMarkingContext(CASheet *sheet, const QString name)
 : CAContext(sheet, name) {
 	_contextType = CAContext::FunctionMarkingContext;
}

CAFunctionMarkingContext::~CAFunctionMarkingContext() {
}

void CAFunctionMarkingContext::clear() {
	for (int i=0; i<_functionMarkingList.size(); i++)
		delete _functionMarkingList[i];
	
	_functionMarkingList.clear();
	_functionMarkingHash.clear();
}

void CAFunctionMarkingContext::addFunctionMarking(CAFunctionMarking *function) {
	int i;
	for (i=_functionMarkingList.size()-1; i>0 && _functionMarkingList[i]->timeStart()>function->timeStart(); i--);
	_functionMarkingList.insert(i+1, function);
	_functionMarkingHash.insert(function->timeStart(), function);
}

CAMusElement *CAFunctionMarkingContext::findNextMusElement(CAMusElement *elt) {
	
}

CAMusElement *CAFunctionMarkingContext::findPrevMusElement(CAMusElement *elt) {
	
}

bool CAFunctionMarkingContext::removeMusElement(CAMusElement *elt, bool cleanup) {
	
}

bool CAFunctionMarkingContext::containsNewFunctionMarking(int timeStart) {
}
