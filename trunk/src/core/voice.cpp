/*
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 *
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#include "core/muselement.h"
#include "core/voice.h"
#include "core/staff.h"
#include "core/clef.h"
#include "core/note.h"
#include "core/rest.h"
#include "core/playable.h"

/*!
	\class CAVoice
	\brief Class which represents a voice in the staff.
	
	CAVoice is a class which holds music elements in the staff. In hieararchy, staff
	includes multiple voices and every voice includes multiple music elements.
	
	\sa CAStaff, CAMusElement
*/

/*!
	Creates a new voice named \a name in parent \a staff.
*/
CAVoice::CAVoice(CAStaff *staff, const QString name) {
	_staff = staff;
	_name = name;
	_voiceNumber = this->staff()->voiceCount() + 1;
	if (_voiceNumber == 1)
		_stemDirection = CANote::StemNeutral;
	else {
		this->staff()->voiceAt(0)->setStemDirection(CANote::StemUp);
		_stemDirection = CANote::StemDown;
	}
	
	_midiChannel = 0;
	_midiProgram = 0;
}

/*!
	Clears and destroys the current voice.
	This also destroys all non-shared music elements held by the voice.
	
	\sa clear()
*/
CAVoice::~CAVoice() {
	clear();
	if (_voiceNumber==2)
		staff()->voiceAt(0)->setStemDirection(CANote::StemNeutral);
}

/*!
	Destroys all non-shared music elements held by the voice.
*/
void CAVoice::clear() {
	while (_musElementList.size()) {
		// deletes an element only if it's not present in other voices or we're deleting the last voice
		if (_musElementList.front()->isPlayable() || staff()->voiceCount()==1)
			delete _musElementList.front(); // CAMusElement's destructor removes it from the list
		else
			_musElementList.removeFirst();
	}
}

/*!
	Inserts the music element \a elt after the last element which startTime is lesser
	or equal to the given \a elt one. Updates start times for elements after it, if
	\a updateT is set. This is false when adding a note to a chord; otherwise true.
	If such an element doesn't exist, appends a music element.
	
	\warning If you're inserting shared signs like barlines, clefs, time signatures and other
	non-playable signs use this method indirectly by calling CAStaff::insertSign(), because
	the mentioned signs MUST be present in *all* voices. Use manually this method only if you
	know what you're doing or you're inserting playable non-shared elements.
	
	\sa insertMusElementBefore(), insertMusElementAfter()
*/
void CAVoice::insertMusElement(CAMusElement *elt, bool updateT) {
	int i;
	for (i=0;
	     (i < _musElementList.size()) && (_musElementList[i]->timeStart() <= elt->timeStart());
	     i++);
	
	_musElementList.insert(i, elt);
	
	if (updateT)
		updateTimes(i);
}

/*!
	Adds a \a note to an already existing \a referenceNote chord or a single note and
	creates a chord out of it.
	Notes in a chord always needs to be sorted by the pitch rising.
	Chord in Canorus isn't its own structure but simply a list of notes sharing the
	same start time.
	
	Returns true, if a referenceNote was found and a note was added; otherwise false.
	
	\sa CANote::chord()
*/
bool CAVoice::addNoteToChord(CANote *note, CANote *referenceNote) {
	int idx = _musElementList.indexOf(referenceNote);
	
	if (idx==-1)
		return false;
	
	QList<CANote*> chord = referenceNote->chord();
	idx = _musElementList.indexOf(chord.first());
	
	int i;
	for (i=0; i<chord.size() && chord[i]->pitch() < note->pitch(); i++);
	
	_musElementList.insert(idx+i, note);
	
	return true;
}

/*!
	Inserts the \a elt right before the given \a eltAfter. If \a eltAfter is 0, it
	appends the element. It updates start times of elements after the given element if
	\a updateTimes is true. If \a force is set, it finds the nearest element with
	larger start time of the given element and inserts the given element before it.
	
	\warning If you're inserting shared signs like barlines, clefs, time signatures and other
	non-playable signs use this method indirectly by calling CAStaff::insertSign(), because
	the mentioned signs MUST be present in *all* voices. Use manually this method only if you
	know what you're doing or you're inserting playable non-shared elements.
	
	Returns true, if \a eltAfter was found and the elt was inserted/appended; otherwise
	false.
*/
bool CAVoice::insertMusElementBefore(CAMusElement *elt, CAMusElement *eltAfter, bool updateT, bool force) {
	if (!eltAfter || !_musElementList.size()) {
		_musElementList << elt;
		return true;
	}
	
	int i;
	for (i=0; (i<_musElementList.size()) && (_musElementList[i] != eltAfter); i++);
	if (i==_musElementList.size())
		if (!force)
			return false;
		else {
			int endTime = eltAfter->timeEnd();
			for (i=0; (i<_musElementList.size()) && (_musElementList[i]->timeStart() < endTime); i++);
			_musElementList.insert(i, elt);
	} else {
		if (_musElementList[i]->musElementType()==CAMusElement::Note &&	// a small check to see if a user wanted to insert an element after a chord, but selected a note in the middle of a chord (not the lowest one)
		    ((CANote*)_musElementList[i])->isPartOfTheChord() &&
		    eltAfter->timeStart()!=elt->timeStart() ) {
			i = _musElementList.indexOf(((CANote*)_musElementList[i])->chord().front());
		}
		_musElementList.insert(i, elt);
	}
	
	if (updateT)
		updateTimes(i);

	return true;
}

/*!
	Inserts the \a elt right after the given \a eltBefore. If \a eltBefore is 0, it
	prepends the element. It updates start times of elements after the given element if
	\a updateTimes is true. If \a force is set, it finds the nearest element with
	smaller start time of the given element and inserts the given element after it.
	
	\warning If you're inserting shared signs like barlines, clefs, time signatures and other
	non-playable signs use this method indirectly by calling CAStaff::insertSign(), because
	the mentioned signs MUST be present in *all* voices. Use manually this method only if you
	know what you're doing or you're inserting playable non-shared elements.
	
	Returns true, if \a eltBefore was found and the elt was inserted/appended;
	otherwise false.
*/
bool CAVoice::insertMusElementAfter(CAMusElement *elt, CAMusElement *eltBefore, bool updateT, bool force) {
	if (!eltBefore || !_musElementList.size()) {
		_musElementList.push_front(elt);
		return true;
	}
	
	int i;
	for (i=0; (i<_musElementList.size()) && (_musElementList[i] != eltBefore); i++);
	if (i==_musElementList.size()) {
		if (!force)
			return false;
		else {
			int endTime = eltBefore->timeEnd();
			for (i=0; (i<_musElementList.size()) && (_musElementList[i]->timeStart() < endTime); i++);
			_musElementList.insert(i, elt);
		}
	} else {
		if (_musElementList[i]->musElementType()==CAMusElement::Note &&	//a small check to see if a user wanted to insert an element after a chord, but selected a note in the middle of a chord (not the lowest one)
		    ((CANote*)_musElementList[i])->isPartOfTheChord() &&
		    eltBefore->timeStart()!=elt->timeStart() ) {
			i = _musElementList.indexOf(((CANote*)_musElementList[i])->chord().back());
		}
		
		_musElementList.insert(++i, elt);
	}
	
	if (updateT)
		updateTimes(i);
	
	return true;
}

/*!
	Returns a pointer to the clef which the given \a elt belongs to.
	Returns 0, if no clefs placed yet.
*/
CAClef* CAVoice::getClef(CAMusElement *elt) {
	CAClef* lastClef = 0;
	for (int i=0; (i<_musElementList.size()) && (_musElementList[i] != elt); i++) {
		if (_musElementList[i]->musElementType() == CAMusElement::Clef)
			lastClef = (CAClef*)_musElementList[i];
	}
	
	return lastClef;
}

/*!
	If the given \a length is 0, take the given music elements \a elt length and
	updates start times of music elements after the given element.
	
	If the given \a length is not 0, updates start time of the given music element \a
	elt and start times of music elements after it for the given length.
	
	Updating times means increasing start time of a music element for certain length.
	
	This function is usually called when an element is deleted or inserted and
	startTimes after it need to be updated.
	
	Returns true, if \a elt was found and update was made; otherwise false;
	
	\sa updateTimesAfter()
*/
bool CAVoice::updateTimes(CAMusElement *elt, int length) {
	int idx;
	if ((idx = _musElementList.indexOf(elt))!=-1) {
		updateTimes(idx, length);
		return true;
	}
		return false;
}

/*!
	Increase start times of music elements after the given \a elt for the \a length.
	
	This function is usually called when an element is deleted or inserted and
	startTimes after it need to be updated.
	
	Returns true, if \a elt was found and update was made; otherwise false;
	
	\sa updateTimesAfter()
*/
bool CAVoice::updateTimesAfter(CAMusElement *elt, int length) {
	int idx;
	if ((idx = _musElementList.indexOf(elt))!=-1) {
		updateTimes(idx+1, length);
		return true;
	}
		return false;
}

/*!
	Updates the musElements timeStarts from index \a idx and on by increasing their
	start times by the \a givenLength.
	
	This function is usually called when an element is deleted or inserted and
	startTimes after it need to be updated.
	
	If \a givenLength is not set, it takes the length of the element at \a idx and
	updates times after it.
*/
void CAVoice::updateTimes(int idx, int givenLength) {
	if (idx >= _musElementList.size())
		return;
	
	int length;
	if (!givenLength)
		length = _musElementList[idx++]->timeLength();
	else
		length = givenLength;
	
	//indent all the music elements after the given index's one.
	//If the music elements aren't connected (previous's end time != next start time)
	for (int i=idx; i<_musElementList.size() && (givenLength ||
	                (_musElementList[i-1]->timeEnd()==_musElementList[i]->timeStart()+length || _musElementList[i-1]->timeStart()==_musElementList[i]->timeStart()+length))
	     ; i++) {
		_musElementList[i]->setTimeStart(_musElementList[i]->timeStart() + length);
	}
}

/*!
	Removes the given music element \a elt from the voice.
	\warning This function doesn't destroy the object, but only removes its
	reference in the voice.
	
	Returns true, if the element was found and removed; otherwise false.
*/
bool CAVoice::removeElement(CAMusElement *elt) {
	int idx;
	if ((idx = _musElementList.indexOf(elt)) != -1) {	//if the search element is found
		int length = _musElementList[idx]->timeLength();
		
		//update the startTimes of elements behind it, but only if the note was not part of the chord
		if (!(elt->musElementType()==CAMusElement::Note && ((CANote*)elt)->isPartOfTheChord()))
			updateTimes(idx+1, -1*length);	//but only, if the removed note is not part of the chord
		
		_musElementList.removeAt(idx);					//remove the element from the music element list
		return true;
	} else {
		return false;
	}
}

/*!
	Returns the pitch of the last note in the voice (default) or of the first note in
	the last chord, if \a inChord is true.
	
	This method is usually used by LilyPond parser when exporting the document, where
	, or ' octave markings need to be determined.
	
	\sa lastPlayableElt()
*/
int CAVoice::lastNotePitch(bool inChord) {
	for (int i=_musElementList.size()-1; i>=0; i--) {
		if (_musElementList[i]->musElementType()==CAMusElement::Note) {
			if (!((CANote*)_musElementList[i])->isPartOfTheChord() || !inChord)	//the note is not part of the chord
				return (((CANote*)_musElementList[i])->pitch());
			else {
				int chordTimeStart = _musElementList[i]->timeStart();	//
				int j;
				for (j=i;
				     (j>=0 && _musElementList[j]->musElementType()==CAMusElement::Note && _musElementList[j]->timeStart()==chordTimeStart);
				     j--);
				
				return (((CANote*)_musElementList[j+1])->pitch());
			}
			
		}
		else if (_musElementList[i]->musElementType()==CAMusElement::Clef)
			return (((CAClef*)_musElementList[i])->centerPitch());
	}

	return -1;
}

/*!
	Returns the last playable element (eg. note or rest) in the voice.
	
	\sa lastNotePitch()
*/
CAPlayable* CAVoice::lastPlayableElt() {
	for (int i=_musElementList.size()-1; i>=0; i--) {
		if (_musElementList[i]->isPlayable())
			return ((CAPlayable*)_musElementList[i]);
	}
	
	return 0;
}

/*!
	Returns true, if this voice contains a note with the given \a pitch at the given
	\a startTime.
	
	This is useful when inserting a note and there needs to be determined if a user is
	adding a note to a chord and the note is maybe already there. Note's accidentals
	are ignored.
*/
bool CAVoice::containsPitch(int pitch, int startTime) {
	 for (int i=0; i<_musElementList.size(); i++)
	 	if ( (_musElementList[i]->musElementType() == CAMusElement::Note) && (_musElementList[i]->timeStart() == startTime) && (((CANote*)_musElementList[i])->pitch() == pitch) )
			return true;
		
	return false;
}

/*!
	Returns a list of pointers to actual music elements which have the given \a
	startTime and are of given \a type.
	This is useful for querying for eg. If a new key signature exists at the certain
	point in time.
*/
QList<CAMusElement*> CAVoice::getEltByType(CAMusElement::CAMusElementType type, int startTime) {
	QList<CAMusElement*> eltList;
	
	int i;
	for (i=0; i < _musElementList.size() && _musElementList[i]->timeStart() < startTime; i++);	// seek to the start of the music elements with the given time
	
	while (i<_musElementList.size() && _musElementList[i]->timeStart()==startTime) {	// create a list of music elements with the given time
		if (_musElementList[i]->musElementType() == type)
			eltList << _musElementList[i];
		i++;
	}
	
	return eltList;
}

/*!
	Returns pointer to the music element before the given \a elt or 0, if the previous
	music element doesn't exist.
*/
CAMusElement *CAVoice::eltBefore(CAMusElement *elt) {
	int idx = _musElementList.indexOf(elt);
	
	if (--idx<0) //if the element wasn't found or was the first element
		return 0;
	
	return _musElementList[idx];
}

/*!
	Returns pointer to the music element after the given \a elt or 0, if the next music
	element doesn't exist.
*/
CAMusElement *CAVoice::eltAfter(CAMusElement *elt) {
	int idx = _musElementList.indexOf(elt);
	
	if (idx==-1) //the element wasn't found
		return 0;
	
	if (++idx==_musElementList.size())	//last element in the list
		return 0;
	
	return _musElementList[idx];
}

/*!
	Returns a list of notes and rests in the given \a time slice (chord) for the
	current voice.
*/
QList<CAPlayable*> CAVoice::getChord(int time) {
	int i;
	for (i=0; i<_musElementList.size() && (_musElementList[i]->timeEnd()<=time || !_musElementList[i]->isPlayable()); i++);
	if (i!=_musElementList.size()) {
		if (_musElementList[i]->musElementType()==CAMusElement::Note) {	// music element is a note
			//! \todo Casting QList<CANote*> to QList<CAPlayable*> doesn't work?! :( Do the conversation manually. This is slow. -Matevz
			QList<CANote*> list = ((CANote*)_musElementList[i])->chord();
			QList<CAPlayable*> ret;
			for (int i=0; i<list.size(); i++)
				ret << list[i];
			return ret;
		}
		else {	// music element is a rest
			QList<CAPlayable*> ret;
			ret << (CARest*)_musElementList[i];
			return ret;
		}
	}
	
	return QList<CAPlayable*>();
}

/*!
	Adds a music element \a elt to the end of the music elements list.
	
	Returns true, if the element was successfully added; false otherwise.
*/
bool CAVoice::appendMusElement(CAMusElement *elt) {
	return insertMusElementBefore(elt, 0, false, false);
}

/*!
	Inserts a music element \a elt at the beginning of the music elements list.
	
	Returns true, if the element was successfully added; false otherwise.
*/
bool CAVoice::prependMusElement(CAMusElement *elt) {
	return insertMusElementAfter(elt, 0, true, false);
}

/*!
	Returns a list of all notes in the voice.
*/
QList<CANote*> CAVoice::noteList() {
	QList<CANote*> list;
	for (int i=0; i<_musElementList.size(); i++)
		if (_musElementList[i]->musElementType()==CAMusElement::Note)
			list << ((CANote*)_musElementList[i]);
	
	return list;
}

/*!
	Sets the stem direction and update slur directions in all the notes in the voice.
*/
void CAVoice::setStemDirection(CANote::CAStemDirection direction) {
	_stemDirection = direction;
}

/*!
	\var CAVoice::_voiceNumber
	Preferred direction of stems for the notes inside the voice. This should be Neutral, if the voice is alone, Up, if the voice is the first voice, Down, if not. Preferred is not used here.

	\sa CANote::CAStemDirection
*/

/*!
	\fn CAVoice::musElementList()
	Returns the list of music elements in the voice.
	
	\sa _musElementList
*/

/*!
	\var CAVoice::_staff
	Staff which this voice belongs to.
	
	\sa staff()
*/

/*!
	\var CAVoice::_voiceNumber
	Voice number in the staff starting at 1.
	
	\sa voiceNumber()
*/
