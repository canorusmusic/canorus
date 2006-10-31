/** @file voice.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "core/muselement.h"
#include "core/voice.h"
#include "core/staff.h"
#include "core/clef.h"

CAVoice::CAVoice(CAStaff *staff, const QString name) {
	_staff = staff;
	_name = name;
	_voiceNumber = this->staff()->voiceCount() + 1;
	
	_midiChannel = 0;
	_midiProgram = 0;
}

void CAVoice::clear() {
	while (_musElementList.size())
		delete _musElementList.front();
}

void CAVoice::insertMusElement(CAMusElement *elt, bool updateT) {
	int i;
	for (i=0;
	     (i < _musElementList.size()) && (_musElementList[i]->timeEnd() <= elt->timeStart());
	     i++);
	
	_musElementList.insert(i, elt);
	
	if (updateT)
		updateTimes(i);
}

bool CAVoice::addNoteToChord(CANote *note, CANote *referenceNote) {
	int idx = _musElementList.indexOf(referenceNote);
	
	if (idx==-1)
		return false;
	
	QList<CANote*> chord = referenceNote->chord();
	idx = _musElementList.indexOf(chord.first());
	
	int i;
	for (i=0; i<chord.size() && chord[i]->pitch() > note->pitch(); i++);

	_musElementList.insert(idx+i, note);
	
	return true;
}

bool CAVoice::insertMusElementBefore(CAMusElement *elt, CAMusElement *eltAfter, bool updateT, bool force) {
	if (!eltAfter || !_musElementList.size()) {
		_musElementList.push_front(elt);
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
		if (_musElementList[i]->musElementType()==CAMusElement::Note &&	//a small check to see if a user wanted to insert an element after a chord, but selected a note in the middle of a chord (not the lowest one)
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

bool CAVoice::insertMusElementAfter(CAMusElement *elt, CAMusElement *eltBefore, bool updateT, bool force) {
	if (!eltBefore || !_musElementList.size()) {
		_musElementList << elt;
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

CAClef* CAVoice::getClef(CAMusElement *elt) {
	CAClef* lastClef = 0;
	for (int i=0; (i<_musElementList.size()) && (_musElementList[i] != elt); i++) {
		if (_musElementList[i]->musElementType() == CAMusElement::Clef)
			lastClef = (CAClef*)_musElementList[i];
	}
		
	return lastClef;
}

bool CAVoice::updateTimes(CAMusElement *elt, int length) {
	int idx;
	if ((idx = _musElementList.indexOf(elt))!=-1) {
		updateTimes(idx, length);
		return true;
	}
		return false;
}

bool CAVoice::updateTimesAfter(CAMusElement *elt, int length) {
	int idx;
	if ((idx = _musElementList.indexOf(elt))!=-1) {
		updateTimes(idx+1, length);
		return true;
	}
		return false;
}

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

CAPlayable* CAVoice::lastPlayableElt() {
	for (int i=_musElementList.size()-1; i>=0; i--) {
		if (_musElementList[i]->isPlayable())
			return ((CAPlayable*)_musElementList[i]);
	}
	
	return 0;
}

bool CAVoice::containsPitch(int pitch, int startTime) {
	 for (int i=0; i<_musElementList.size(); i++)
	 	if ( (_musElementList[i]->musElementType() == CAMusElement::Note) && (_musElementList[i]->timeStart() == startTime) && (((CANote*)_musElementList[i])->pitch() == pitch) )
	 		return true;
	 
	 return false;
}

QList<CAMusElement*> CAVoice::getEltByType(CAMusElement::CAMusElementType type, int startTime) {
	QList<CAMusElement*> eltList;
	
	int i;
	for (i=0; i < _musElementList.size() && _musElementList[i]->timeStart() < startTime; i++);	//seek to the start of the music elements with the given time
	
	while (i<_musElementList.size() && _musElementList[i]->timeStart()==startTime) {	//create a list of music elements with the given time
		if (_musElementList[i]->musElementType() == type)
			eltList << _musElementList[i];
		i++;
	}
	
	return eltList;
}

CAMusElement *CAVoice::eltBefore(CAMusElement *elt) {
	int idx = _musElementList.indexOf(elt);
	
	if (--idx<0) //if the element wasn't found or was the first element
		return 0;
	
	return _musElementList[idx];
}

CAMusElement *CAVoice::eltAfter(CAMusElement *elt) {
	int idx = _musElementList.indexOf(elt);
	
	if (idx==-1) //the element wasn't found
		return 0;
	
	if (++idx==_musElementList.size())	//last element in the list
		return 0;
	
	return _musElementList[idx];
}
#include <iostream>
QList<CANote*> CAVoice::getChord(int time) {
	int i;
	for (i=0; i<_musElementList.size() && (_musElementList[i]->timeEnd()<=time || _musElementList[i]->musElementType()!=CAMusElement::Note); i++);
	if (i!=_musElementList.size()) {
		return ((CANote*)_musElementList[i])->chord();
	}
	
	return QList<CANote*>();
}
