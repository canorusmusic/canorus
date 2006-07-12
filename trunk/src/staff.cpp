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
	_type = CAContext::CAStaff;
	_numberOfLines = 5;
}

void CAStaff::clear() {
	for (int i=0; i<_voiceList.size(); i++) {
		_voiceList[i]->clear();
		delete _voiceList[i];
	}
	
	_voiceList.clear();
}
