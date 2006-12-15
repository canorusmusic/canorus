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

int CABarline::compare(CAMusElement *elt) {
	if (elt->musElementType()!=CAMusElement::Barline)
		return -1;
	
	int diffs=0;
	if (_barlineType!=((CABarline*)elt)->barlineType()) diffs++;
	
	return diffs;
}

const QString CABarline::barlineTypeToString(CABarlineType type) {
	switch (type) {
		case Single: return "single";
		case Double: return "double";
		case End: return "end";
		case RepeatOpen: return "repeat-open";
		case RepeatClose: return "repeat-close";
		case Dotted: return "dotted";
		default: return "";
	}
}

CABarline::CABarlineType CABarline::barlineTypeFromString(const QString type) {
	if (type=="single") return Single; else
	if (type=="double") return Double; else
	if (type=="end") return End; else
	if (type=="repeat-open") return RepeatOpen; else
	if (type=="repeat-close") return RepeatClose; else
	if (type=="dotted") return Dotted;
	else return Single;	
}
