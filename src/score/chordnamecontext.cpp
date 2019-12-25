/*!
	Copyright (c) 2019, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

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
void CAChordNameContext::addChordName( CAFiguredBassMark *m, bool replace ) {
	int i;
	for (i=0; i<_chordNameList.size() && _chordNameList[i]->timeStart()<m->timeStart(); i++);
	//int s = _figuredBassMarkList.size();
	if ( i<_chordNameList.size() && replace ) {
		delete _chordNameList.takeAt(i);
	}
	_chordNameList.insert(i, m);
	for (i++; i<_chordNameList.size(); i++)
		_chordNameList[i]->setTimeStart( _chordNameList[i]->timeStart() + m->timeLength() );
}

/*!
	Inserts an empty chord name and shifts the chord names after.
	This function is usually called when initializing the context.
*/
void CAChordNameContext::addEmptyChordName( int timeStart, int timeLength ) {
	int i;
	for (i=0; i<_chordNameList.size() && _chordNameList[i]->timeStart()<timeStart; i++);
	_chordNameList.insert(i, (new CAChordName( this, timeStart, timeLength )));
	for (i++; i<_chordNameList.size(); i++)
		_chordNameList[i]->setTimeStart( _chordNameList[i]->timeStart() + timeLength );
}

/*!
	Updates timeStarts and timeLength of all figured bass marks according to the chords they belong.
	Adds new empty figured bass marks at the end, if needed.
 */
void CAChordNameContext::repositChordNames() {
	if ( !sheet() ) {
		return;
	}

	QList<CAPlayable*> chord = sheet()->getChord(0);
	int fbmIdx = 0;
	while (chord.size()) {
		int maxTimeStart = chord[0]->timeStart();
		int minTimeEnd = chord[0]->timeEnd();
		bool notes = false; // are notes present in the chord or only rests?
		for (int i=1; i<chord.size(); i++) {
			if (chord[i]->musElementType()==CAMusElement::Note) {
				notes = true;
			}

			if (chord[i]->timeStart() > maxTimeStart) {
				maxTimeStart = chord[i]->timeStart();
			}
			if (chord[i]->timeEnd() < minTimeEnd) {
				minTimeEnd = chord[i]->timeEnd();
			}
		}

		// only assign figured bass marks under the notes
		if (notes) {
			// add new empty chord name, if none exist
			if ( fbmIdx==_chordNameList.size() ) {
				addEmptyChordName( maxTimeStart, minTimeEnd-maxTimeStart );
			}

			CAChordName *chordName = _chordNameList[fbmIdx];
			chordName->setTimeStart( maxTimeStart );
			chordName->setTimeLength( minTimeEnd-maxTimeStart );
			fbmIdx++;
		}

		chord = sheet()->getChord(minTimeEnd);
	}

	// updated times for the chord names at the end (after the score)
	for (; fbmIdx < _chordNameList.size(); fbmIdx++) {
		_chordNameList[fbmIdx]->setTimeStart(((fbmIdx>0)?_chordNameList[fbmIdx-1]:_chordNameList[0])->timeEnd());
		_chordNameList[fbmIdx]->setTimeLength(CAPlayableLength::Quarter);
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

	int i = _chordNameList.indexOf(static_cast<CAChordNAme*>(elt));
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
