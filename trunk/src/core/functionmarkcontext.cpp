/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/sheet.h"
#include "core/playable.h"
#include "core/functionmarkcontext.h"
#include "core/functionmark.h"

/*!
	\class CAFunctionMarkContext
	\brief Context for function marks
	
	This class represents a context which holds various function marks.
	As CAStaff is a parent context for CANote, CARest and other staff elements,
	CAFunctionMarkContext is a parent context for CAFunctionMark.
	
	\sa CAContext
*/

CAFunctionMarkContext::CAFunctionMarkContext( const QString name, CASheet *sheet )
 : CAContext( name, sheet ) {
 	_contextType = CAContext::FunctionMarkContext;
 	
 	repositFunctions();
}

CAFunctionMarkContext::~CAFunctionMarkContext() {
	clear();
}

CAFunctionMarkContext *CAFunctionMarkContext::clone( CASheet *s ) {
	CAFunctionMarkContext *newFmc = new CAFunctionMarkContext( name(), s );
	
	for (int i=0; i<_functionMarkList.size(); i++) {
		CAFunctionMark *newFm = _functionMarkList[i]->clone();
		newFm->setContext( newFmc );
		newFmc->addFunctionMark( newFm );
	}
	
	return newFmc;
}

void CAFunctionMarkContext::clear() {
	for (int i=0; i<_functionMarkList.size(); i++)
		delete _functionMarkList[i];
	
	_functionMarkList.clear();
}

/*!
	Adds an already created function mark to this context.
*/
void CAFunctionMarkContext::addFunctionMark( CAFunctionMark *function, bool replace ) {
	int i;
	for (i=_functionMarkList.size()-1; i>0 && _functionMarkList[i]->timeStart()>function->timeStart(); i--);
	_functionMarkList.insert( i+1, function );
	if ( replace && i<_functionMarkList.size() && i>=0 && _functionMarkList[i]->isEmpty() ) {
		_functionMarkList.removeAt( i );
	} else if (!replace) {
		i++;
		while ( ++i < _functionMarkList.size() )
			_functionMarkList[i]->setTimeStart( _functionMarkList[i]->timeStart() + function->timeLength() );
	}
}

CAMusElement *CAFunctionMarkContext::next(CAMusElement *elt) {
	int idx = _functionMarkList.indexOf( static_cast<CAFunctionMark*>(elt) );
	if (idx==-1)
		return 0;
	
	if (++idx>=_functionMarkList.size())
		return 0;
	else
		return _functionMarkList[idx];
}

CAMusElement *CAFunctionMarkContext::previous(CAMusElement *elt) {
	int idx = _functionMarkList.indexOf( static_cast<CAFunctionMark*>(elt) );
	if (idx==-1)
		return 0;
	
	if (--idx<0)
		return 0;
	else
		return _functionMarkList[idx];	
}

bool CAFunctionMarkContext::remove( CAMusElement *elt ) {
	return _functionMarkList.removeAll(static_cast<CAFunctionMark*>(elt));
}

/*!
	This method is similar to CALyircsContext::repositFunctions().
	It repositions the functions (sets timeStart and timeLength) one by one according to the chords
	above the context.
	
	If two functions contain the same timeStart, they are treated as modulation and will contain
	the same timeStart after reposition is done as well!
*/
void CAFunctionMarkContext::repositFunctions() {
	int TS, TL;
	int curIdx;
	QList<CAPlayable*> chord;
	for ( TS=0, curIdx=0; (chord=sheet()->getChord(TS)).size() || curIdx<_functionMarkList.size(); TS+=TL ) {
		TL = (chord.size()?chord[0]->timeLength():256);
		for ( int i=0; i<chord.size(); i++ )
			if (chord[i]->timeLength()<TL)
				TL = chord[i]->timeLength();
		
		if ( curIdx == _functionMarkList.size() ) { // add new empty functions, if chords still exist
			addEmptyFunction( TS, TL);
			curIdx++;
		}
		
		// apply timestart and length to existing function marks
		for ( int startIdx = curIdx; curIdx==0 || curIdx < _functionMarkList.size() && _functionMarkList[curIdx]->timeStart()==_functionMarkList[startIdx]->timeStart(); curIdx++ ) {
			_functionMarkList[curIdx]->setTimeLength( TL );
			_functionMarkList[curIdx]->setTimeStart( TS );
		}
	}
}

/*!
	Adds an undefined function mark (uses for empty function marks when only function mark context exists and no actual
	functions added).
*/
void CAFunctionMarkContext::addEmptyFunction( int timeStart, int timeLength ) {
	addFunctionMark( new CAFunctionMark( CAFunctionMark::Undefined, false, "C", this, timeStart, timeLength ), false );
}

/*!
	Returns number of function marks with the given \a timeStart.
*/
int CAFunctionMarkContext::functionMarkCount(int timeStart) {
	int i;
	for (i=0; i<_functionMarkList.size() && _functionMarkList[i]->timeStart()>timeStart; i++);
	if (i<_functionMarkList.size() && _functionMarkList[i]->timeStart()==timeStart) {
		int count;
		for (count=1; i<_functionMarkList.size() && _functionMarkList[i]->timeStart()==timeStart; count++, i++);
		return count;
	} else
		return 0;
}

/*!
	\var CAFunctionMarkContext::_functionMarkList
	List of all the function marks sorted by timeStart
*/
