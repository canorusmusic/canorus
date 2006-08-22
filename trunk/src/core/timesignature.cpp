/** @file timesignature.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "core/timesignature.h"
#include "core/staff.h"

CATimeSignature::CATimeSignature(int beats, int beat, CAStaff *staff, int startTime, CATimeSignatureType type)
 : CAMusElement(staff, startTime) {
 	_musElementType = CAMusElement::TimeSignature;
 	
 	_beats = beats;
 	_beat = beat;
 	_timeSignatureType = type;
}

CATimeSignature::~CATimeSignature() {
}
