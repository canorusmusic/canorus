/** @file muselement.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "core/muselement.h"
#include "core/context.h"
#include "core/staff.h"

CAMusElement::CAMusElement(CAContext *context, int time, int length) {
	_context = context;
	_timeStart = time;
	_timeLength = length;
	_playable = false;
}

CAMusElement::~CAMusElement() {
}
