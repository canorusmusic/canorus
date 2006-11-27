/** @file core/functionmarking.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "core/functionmarking.h"
#include "core/functionmarkingcontext.h"

CAFunctionMarking::CAFunctionMarking(CAFunctionType function, const QString key, CAFunctionMarkingContext* context, int timeStart, int timeLength, bool minor, CAFunctionType chordArea, bool chordAreaMinor, CAFunctionType tonicDegree, bool tonicDegreeMinor, const QString alterations, bool ellipseSequence)
 : CAMusElement(context, timeStart, timeLength) {
 	_musElementType = CAMusElement::FunctionMarking;
 	
 	_function = function;
 	_tonicDegree = tonicDegree;
 	_tonicDegreeMinor = tonicDegreeMinor;
 	_key = key;
	_chordArea = chordArea;
	_chordAreaMinor = chordAreaMinor;
	_minor = minor;
	_ellipseSequence = ellipseSequence;
	
	setAlterations(alterations);
}

CAFunctionMarking::~CAFunctionMarking() {
}

bool CAFunctionMarking::isSideDegree() {
	if (_function==I ||
	    _function==II ||
	    _function==III ||
	    _function==IV ||
	    _function==V ||
	    _function==VI ||
	    _function==VII)
		return true;
	else
		return false;
}

CAFunctionMarking *CAFunctionMarking::clone() {
	CAFunctionMarking *newElt;
	newElt = new CAFunctionMarking(function(), key(), (CAFunctionMarkingContext*)_context, timeStart(), timeLength(), isMinor(), chordArea(), isChordAreaMinor(), tonicDegree(), isTonicDegreeMinor(), "", isPartOfEllipse());
	newElt->setAlteredDegrees(_alteredDegrees);
	newElt->setAddedDegrees(_addedDegrees);
	
	return newElt;
}

int CAFunctionMarking::compare(CAMusElement *func) {
	return 0;	//TODO
}

void CAFunctionMarking::setAlterations(const QString alterations) {
	if (alterations.isEmpty())
		return;
	
	int i=0;	//index of the first character that belongs to the degree
	int rightIdx;
	
	//added degrees:
	_addedDegrees.clear();
	while (i<alterations.size() && alterations[i]!='+' && alterations[i]!='-') {
		if (alterations.indexOf('+',i+1)==-1)
			rightIdx = alterations.indexOf('-',i+1);
		else if (alterations.indexOf('-',i+1)==-1)
			rightIdx = alterations.indexOf('+',i+1);
		else
			rightIdx = alterations.indexOf('+',i+1)<alterations.indexOf('-',i+1)?alterations.indexOf('+',i+1):alterations.indexOf('-',i+1);
		
		QString curDegree = alterations.mid(i, rightIdx-i+1);
		curDegree.insert(0, curDegree[curDegree.size()-1]);	//move the last + or - before the string
		curDegree.chop(1);
		_addedDegrees << curDegree.toInt();
		i=rightIdx+1;
	}
	
	//altered degrees:
	_alteredDegrees.clear();
	while (i<alterations.size()) {
		if (alterations.indexOf('+',i+1)==-1 && alterations.indexOf('-',i+1)!=-1)
			rightIdx = alterations.indexOf('-',i+1);
		else if (alterations.indexOf('-',i+1)==-1 && alterations.indexOf('+',i+1)!=-1)
			rightIdx = alterations.indexOf('+',i+1);
		else if (alterations.indexOf('-',i+1)!=-1 && alterations.indexOf('+',i+1)!=-1)
			rightIdx = alterations.indexOf('+',i+1)<alterations.indexOf('-',i+1)?alterations.indexOf('+',i+1):alterations.indexOf('-',i+1);
		else
			rightIdx = alterations.size();
		
		_alteredDegrees << alterations.mid(i, rightIdx-i).toInt();
		i=rightIdx;
	}
}
