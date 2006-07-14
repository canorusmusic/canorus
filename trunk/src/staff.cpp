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

CAClef* CAStaff::insertClef(CAClef::CAClefType clefType, int timeStart) {
	CAClef *clef = new CAClef(clefType, this, timeStart);
	
	for (int i=0; i<_voiceList.size(); i++)
		_voiceList[i]->insertClef(clef);
	
	return clef;
}

CAClef *CAStaff::insertClefBefore(CAClef::CAClefType clefType, CAMusElement *eltAfter) {
	CAClef *clef = new CAClef(clefType, this, (eltAfter?eltAfter->timeStart():lastTimeEnd()));
	
	for (int i=0; i<_voiceList.size(); i++) {
		if (!_voiceList[i]->insertClefBefore(clef, eltAfter))
			std::cerr << "Internal error: CAStaff::insertClefBefore() - eltAfter does not exist!" << std::endl;
	}
	
	return clef;
}
