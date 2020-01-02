/*!
	Copyright (c) 2019, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/
#include <QDebug>

#include "score/chordnamecontext.h"
#include "score/chordname.h"
#include "score/sheet.h"
#include "score/playable.h"
#include "score/playablelength.h"

/*!
	\class CAChordNameContext
	\brief Context for chord names

	This class represents a container for the chord names.
	It is somehow similar to CALyricsContext in terms of use.
	The class keeps all chord names in a single list. No specific times are stored.
	This is because exactly one chord name is assigned to every chord in the score.
 */

CAChordNameContext::CAChordNameContext( QString name, CASheet *sheet )
 : CAContext(name, sheet) {
	setContextType( ChordNameContext );
	repositChordNames();
}

CAChordNameContext::~CAChordNameContext() {
	clear();
}

/*!
	Inserts the given chord name \a m according to its timeStart.
	Replaces any existing chord name at that time, if \a replace is True (default).
 */
void CAChordNameContext::addChordName( CAChordName *m, bool replace ) {
	int i;
	for (i=0; i<_chordNameList.size() && _chordNameList[i]->timeStart()<m->timeStart(); i++);
	if ( i<_chordNameList.size() && replace ) {
		delete _chordNameList.takeAt(i);
	}
	_chordNameList.insert(i, m);
	for (i++; i<_chordNameList.size(); i++) {
		_chordNameList[i]->setTimeStart( _chordNameList[i]->timeStart() + m->timeLength() );
	}
}

/*!
	Inserts an empty chord name and shifts the chord names after.
	This function is usually called when initializing the context.
*/
void CAChordNameContext::addEmptyChordName( int timeStart, int timeLength ) {
	int i;
	for (i=0; i<_chordNameList.size() && _chordNameList[i]->timeStart()<timeStart; i++);
	_chordNameList.insert(i, (new CAChordName( CADiatonicPitch::Undefined, "", this, timeStart, timeLength )));
	for (i++; i<_chordNameList.size(); i++)
		_chordNameList[i]->setTimeStart( _chordNameList[i]->timeStart() + timeLength );
}

/*!
	It repositions the existing chord names (sets timeStart and timeLength) one by one according to the playable music
	elements above the context.

 	\sa CALyricsContext::repositSyllables(), CAFiguredBassContext::repositFiguredBassMarks(), CAFunctionMarkContext::repositFunctions()
*/
void CAChordNameContext::repositChordNames() {
	int ts, tl;
	int curIdx; // contains current position in _chordNameList
	QList<CAPlayable *> chord;
	for (ts = 0, curIdx = 0;
		 (sheet() && (chord = sheet()->getChord(ts)).size()) || curIdx < _chordNameList.size(); ts += tl, curIdx++) {
		tl = (chord.size() ? chord[0]->timeLength() : 256);
		for (int i = 0; i < chord.size(); i++)
			if (chord[i]->timeLength() < tl)
				tl = chord[i]->timeLength();

		// add new empty chord names, if playables still exist above
		if (curIdx == _chordNameList.size()) {
			addEmptyChordName(ts, tl);
		}

		// apply timeStart and timeLength to existing chord names
		if (curIdx < _chordNameList.size()) {
			_chordNameList[curIdx]->setTimeLength(tl);
			_chordNameList[curIdx]->setTimeStart(ts);
		}
	}
}

/*!
	Returns chord name at the given \a time.
 */
CAChordName *CAChordNameContext::chordNameAtTimeStart( int time ) {
	int i;
	for ( i=0; i<_chordNameList.size() && _chordNameList[i]->timeStart() <= time; i++);
	if (i>0 && _chordNameList[--i]->timeEnd()>time) {
		return _chordNameList[i];
	} else {
		return 0;
	}
}

CAContext* CAChordNameContext::clone( CASheet* s ) {
	CAChordNameContext *newCnc = new CAChordNameContext( name(), s );

	for (int i=0; i<_chordNameList.size(); i++) {
		CAChordName *newCn = static_cast<CAChordName*>(_chordNameList[i]->clone(newCnc));
		newCnc->addChordName( newCn );
	}
	return newCnc;
}

void CAChordNameContext::clear() {
	while(!_chordNameList.isEmpty())
		delete _chordNameList.takeFirst();
}

CAMusElement* CAChordNameContext::next( CAMusElement* elt ) {
	if (elt->musElementType()!=CAMusElement::ChordName)
		return 0;

	int i = _chordNameList.indexOf(static_cast<CAChordName*>(elt));
	if (i!=-1 && ++i<_chordNameList.size())
		return _chordNameList[i];
	else
		return 0;
}

CAMusElement* CAChordNameContext::previous( CAMusElement* elt ) {
	if (elt->musElementType()!=CAMusElement::ChordName)
		return 0;

	int i = _chordNameList.indexOf(static_cast<CAChordName*>(elt));
	if (i!=-1 && --i>-1)
		return _chordNameList[i];
	else
		return 0;
}

bool CAChordNameContext::remove( CAMusElement *elt ) {
	if (!elt || elt->musElementType()!=CAMusElement::ChordName)
		return false;

	bool success=false;
	success = _chordNameList.removeAll(static_cast<CAChordName*>(elt));

	if(success)
		delete elt;

	return success;
}
