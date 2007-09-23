/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/sheet.h"
#include "core/playable.h"
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

CAFunctionMarkingContext::CAFunctionMarkingContext( const QString name, CASheet *sheet )
 : CAContext( name, sheet ) {
 	_contextType = CAContext::FunctionMarkingContext;
 	
 	repositFunctions();
}

CAFunctionMarkingContext::~CAFunctionMarkingContext() {
	clear();
}

CAFunctionMarkingContext *CAFunctionMarkingContext::clone( CASheet *s ) {
	CAFunctionMarkingContext *newFmc = new CAFunctionMarkingContext( name(), s );
	
	for (int i=0; i<_functionMarkingList.size(); i++) {
		CAFunctionMarking *newFm = _functionMarkingList[i]->clone();
		newFm->setContext( newFmc );
		newFmc->addFunctionMarking( newFm );
	}
	
	return newFmc;
}

void CAFunctionMarkingContext::clear() {
	for (int i=0; i<_functionMarkingList.size(); i++)
		delete _functionMarkingList[i];
	
	_functionMarkingList.clear();
}

/*!
	Adds an already created function marking to this context.
*/
void CAFunctionMarkingContext::addFunctionMarking( CAFunctionMarking *function, bool replace ) {
	int i;
	for (i=_functionMarkingList.size()-1; i>0 && _functionMarkingList[i]->timeStart()>function->timeStart(); i--);
	_functionMarkingList.insert( i+1, function );
	if ( replace && i<_functionMarkingList.size() && i>=0 && _functionMarkingList[i]->isEmpty() ) {
		_functionMarkingList.removeAt( i );
	} else if (!replace) {
		i++;
		while ( ++i < _functionMarkingList.size() )
			_functionMarkingList[i]->setTimeStart( _functionMarkingList[i]->timeStart() + function->timeLength() );
	}
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
	
	return _functionMarkingList.removeAll(static_cast<CAFunctionMarking*>(elt));
}

/*!
	This method is similar to CALyircsContext::repositFunctions().
	It repositions the functions (sets timeStart and timeLength) one by one according to the chords
	above the context.
	
	If two functions contain the same timeStart, they are treated as modulation and will contain
	the same timeStart after reposition is done as well!
*/
void CAFunctionMarkingContext::repositFunctions() {
	int TS, TL;
	int curIdx;
	QList<CAPlayable*> chord;
	for ( TS=0, curIdx=0; (chord=sheet()->getChord(TS)).size() || curIdx<_functionMarkingList.size(); TS+=TL ) {
		TL = (chord.size()?chord[0]->timeLength():256);
		for ( int i=0; i<chord.size(); i++ )
			if (chord[i]->timeLength()<TL)
				TL = chord[i]->timeLength();
		
		if ( curIdx == _functionMarkingList.size() ) { // add new empty functions, if chords still exist
			addEmptyFunction( TS, TL);
			curIdx++;
		}
		
		// apply timestart and length to existing function markings
		for ( int startIdx = curIdx; curIdx==0 || curIdx < _functionMarkingList.size() && _functionMarkingList[curIdx]->timeStart()==_functionMarkingList[startIdx]->timeStart(); curIdx++ ) {
			_functionMarkingList[curIdx]->setTimeLength( TL );
			_functionMarkingList[curIdx]->setTimeStart( TS );
		}
	}
}

/*!
	Adds an undefined function marking (uses for empty function markings when only function marking context exists and no actual
	functions added).
*/
void CAFunctionMarkingContext::addEmptyFunction( int timeStart, int timeLength ) {
	addFunctionMarking( new CAFunctionMarking( CAFunctionMarking::Undefined, false, "C", this, timeStart, timeLength ), false );
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
