/** @file core/functionmarking.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "core/functionmarking.h"
#include "core/functionmarkingcontext.h"

CAFunctionMarking::CAFunctionMarking(CADegreeType degree, QString key, CAFunctionMarkingContext* context, int timeStart, int timeLength)
 : CAMusElement(context, timeStart, timeLength) {
 	_musElementType = CAMusElement::FunctionMarking;
 	
 	_degree = degree;
 	_key = key;
}

CAFunctionMarking::~CAFunctionMarking() {
}

bool CAFunctionMarking::isSideDegree() {
	if (_degree==I ||
	    _degree==II ||
	    _degree==III ||
	    _degree==IV ||
	    _degree==V ||
	    _degree==VI ||
	    _degree==VII)
		return true;
	else
		return false;
}
