/** @file core/barline.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "core/barline.h"
#include "core/staff.h"

CABarline::CABarline(CABarlineType type, CAStaff *staff, int startTime) 
 : CAMusElement(staff, startTime) {
 	_musElementType = CAMusElement::Barline;
 	_barlineType = type;
}

CABarline::~CABarline() {
}

CABarline* CABarline::clone() {
	return new CABarline(_barlineType, (CAStaff*)_context, _timeStart);
}
