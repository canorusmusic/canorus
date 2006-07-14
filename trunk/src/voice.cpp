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
	_voiceNumber = 1;
	_staff = staff;
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
