/** @file staff.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QPainter>
#include <iostream>

#include "voice.h"
#include "staff.h"
#include "note.h"

CAStaff::CAStaff(CASheet *s) : CAContext(s) {
	_contextType = CAContext::Staff;
	_numberOfLines = 5;
	
	_voiceList << new CAVoice(this);
}

int CAStaff::lastTimeEnd() {
	int maxTime = 0;
	for (int i=0; i<_voiceList.size(); i++)
		if (_voiceList[i]->lastTimeEnd() > maxTime)
			maxTime = _voiceList[i]->lastTimeEnd();
	
	return maxTime;
}

void CAStaff::clear() {
	for (int i=0; i<_voiceList.size(); i++) {
		_voiceList[i]->clear();
		delete _voiceList[i];
	}
	
	_voiceList.clear();
}

void CAStaff::insertSign(CAMusElement *sign) {
	for (int i=0; i<_voiceList.size(); i++)
		_voiceList[i]->insertMusElement(sign);
}

bool CAStaff::insertSignBefore(CAMusElement *sign, CAMusElement *eltAfter) {
	bool error = false;
	for (int i=0; i<_voiceList.size(); i++) {
		if (!_voiceList[i]->insertMusElementBefore(sign, eltAfter))
			error = true;
	}
	
	return (!error);
}

void CAStaff::insertNote(CANote *note) {
	note->voice()->insertMusElement(note);
}

bool CAStaff::insertNoteBefore(CANote *note, CAMusElement *eltAfter) {
	return note->voice()->insertMusElementBefore(note, eltAfter);
}

bool CAStaff::removeMusElement(CAMusElement *elt) {
	bool success = false;
	for (int i=0; i<_voiceList.size(); i++)
		if (_voiceList[i]->removeElement(elt))
			success = true;
	
	return success;
}
