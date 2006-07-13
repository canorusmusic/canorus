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

void CAVoice::addClef(CAClef *clef) {
	int i;
	for (i=0;
	     (i < _musElementList.size()) && (_musElementList[i]->timeStart() > clef->timeStart());
	     i++);
	
	_musElementList.insert(i, clef);
}
