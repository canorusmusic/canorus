/* 
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#include "core/functionmarkingcontext.h"
#include "core/functionmarking.h"

/*!
	\class CAFunctionMarkingContext
	\brief Context for function markings
	
	This class represents a context which holds various function markings.
	As CAStaff is a parent context for CANote, CARest and other staff elements,
	CAFunctionMarkingContext is a parent context for CAFunctionMarking.
	
	\sa CAContext
*/

CAFunctionMarkingContext::CAFunctionMarkingContext(CASheet *sheet, const QString name)
 : CAContext(sheet, name) {
 	_contextType = CAContext::FunctionMarkingContext;
}

CAFunctionMarkingContext::~CAFunctionMarkingContext() {
	clear();
}

void CAFunctionMarkingContext::clear() {
	for (int i=0; i<_functionMarkingList.size(); i++)
		delete _functionMarkingList[i];
	
	_functionMarkingList.clear();
	_functionMarkingHash.clear();
}

/*!
	Adds an already created function marking to this context.
*/
void CAFunctionMarkingContext::addFunctionMarking(CAFunctionMarking *function) {
	int i;
	for (i=_functionMarkingList.size()-1; i>0 && _functionMarkingList[i]->timeStart()>function->timeStart(); i--);
	_functionMarkingList.insert(i+1, function);
	_functionMarkingHash.insertMulti(function->timeStart(), function);
	function->setContext(this);
}

CAMusElement *CAFunctionMarkingContext::findNextMusElement(CAMusElement *elt) {
	int idx = _functionMarkingList.indexOf((CAFunctionMarking*)elt);
	if (idx==-1)
		return 0;
	
	if (++idx>=_functionMarkingList.size())
		return 0;
	else
		return _functionMarkingList[idx];
}

CAMusElement *CAFunctionMarkingContext::findPrevMusElement(CAMusElement *elt) {
	int idx = _functionMarkingList.indexOf((CAFunctionMarking*)elt);
	if (idx==-1)
		return 0;
	
	if (--idx<0)
		return 0;
	else
		return _functionMarkingList[idx];	
}

bool CAFunctionMarkingContext::removeMusElement(CAMusElement *elt, bool cleanup) {
	if (cleanup)
		delete elt;
	
	return _functionMarkingList.removeAll((CAFunctionMarking*)elt);
}

/*!
	Returns number of function markings with the given \a timeStart.
*/
int CAFunctionMarkingContext::functionMarkingCount(int timeStart) {
	int i;
	for (i=0; i<_functionMarkingList.size() && _functionMarkingList[i]->timeStart()>timeStart; i++);
	if (i<_functionMarkingList.size() && _functionMarkingList[i]->timeStart()==timeStart) {
		int count;
		for (count=1; i<_functionMarkingList.size() && _functionMarkingList[i]->timeStart()==timeStart; count++, i++);
		return count;
	} else
		return 0;
}

/*!
	\var CAFunctionMarkingContext::_functionMarkingList
	List of all the function markings sorted by timeStart
*/

/*!
	\var CAFunctionMarkingContext::_functionMarkingHash
	Map of all the function markings in certain time slice - used by
	containsNewFunctionMarking().
*/
