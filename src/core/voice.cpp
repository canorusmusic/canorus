/** @file voice.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "muselement.h"
#include "voice.h"
#include "staff.h"
#include "clef.h"

CAVoice::CAVoice(CAStaff *staff) {
	_staff = staff;
	_voiceNumber = this->staff()->voiceCount() + 1;
	
	_midiChannel = 0;
	_midiProgram = 0;
}

void CAVoice::clear() {
	for (int i=0; i<_musElementList.size(); i++) {
		delete _musElementList[i];
	}
	
	_musElementList.clear();
}

void CAVoice::insertMusElement(CAMusElement *elt) {
	int i;
	for (i=0;
	     (i < _musElementList.size()) && (_musElementList[i]->timeStart() < elt->timeStart());
	     i++);
	
	_musElementList.insert(i, elt);
	
	updateTimes(i);
}

bool CAVoice::insertMusElementBefore(CAMusElement *elt, CAMusElement *eltAfter) {
	if (!eltAfter) {
		_musElementList << elt;
		return true;
	}
	
	int i;
	for (i=0; (i<_musElementList.size()) && (_musElementList[i] != eltAfter); i++);
	if (i==_musElementList.size())
		return false;
	
	_musElementList.insert(i, elt);
	
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

void CAVoice::updateTimes(int idx) {
	int length = _musElementList[idx]->timeLength();
	for (int i=idx+1; i<_musElementList.size(); i++) {
		_musElementList[i]->setTimeStart(_musElementList[i]->timeStart() + length);
	}
}

void CAVoice::updateTimes(int idx, int length) {
	for (int i=idx; i<_musElementList.size(); i++) {
		_musElementList[i]->setTimeStart(_musElementList[i]->timeStart() - length);
	}
}

bool CAVoice::removeElement(CAMusElement *elt) {
	int idx;
	if ((idx = _musElementList.indexOf(elt)) != -1) {	//if the search element is found
		int length = _musElementList[idx]->timeLength();
		_musElementList.removeAt(idx);					//remove it
		if (idx < _musElementList.size())				//update the startTimes of elements behind it
			updateTimes(idx, length);
		return true;
	} else {
		return false;
	}
}
