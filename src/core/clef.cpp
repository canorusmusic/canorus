/** @file clef.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "core/clef.h"
#include "core/staff.h"

CAClef::CAClef(CAClefType type, CAStaff *staff, int time) : CAMusElement(staff, time) {
	_musElementType = CAMusElement::Clef;
	setClefType(type);
}

CAClef::CAClef(const QString type, CAStaff *staff, int time) : CAMusElement(staff, time) {
	_musElementType = CAMusElement::Clef;
	
	CAClefType clefType;
	if (type=="treble") {
		clefType = Treble;
	} else if (type=="bass") {
		clefType = Bass;
	} else if (type=="alto") {
		clefType = Alto;
	} else if (type=="tenor") {
		clefType = Tenor;
	} else if (type=="soprano") {
		clefType = Soprano;
	}
	
	setClefType(clefType);
}

void CAClef::setClefType(CAClefType type) {
	_clefType = type;
	
	switch (type) {
		case Treble:
			_c1 = -2;
			_centerPitch = 32;
			break;
		case Bass:
			_c1 = 10;
			_centerPitch = 24;
			break;
		case Alto:
			_c1 = 4;
			_centerPitch = 28;
			break;
		case Tenor:
			_c1 = 6;
			_centerPitch = 28;
			break;
		case Soprano:
			_c1 = 0;
			_centerPitch = 28;
			break;
	}
}

CAClef* CAClef::clone() {
	return new CAClef(_clefType, (CAStaff*)_context, _timeStart);
}

const QString CAClef::clefTypeML() {
	QString type;
	
	switch (_clefType) {
		case Treble:
			type = "treble";
			break;
		case Bass:
			type = "bass";
			break;
		case Alto:
			type = "alto";
			break;
		case Tenor:
			type = "tenor";
			break;
		case Soprano:
			type = "soprano";
			break;
	}
	
	return type;
}

int CAClef::compare(CAMusElement *elt) {
	if (elt->musElementType()!=CAMusElement::Clef)
		return -1;
	
	int diffs=0;
	if (_clefType!=((CAClef*)elt)->clefType()) diffs++;
		
	return diffs;
}
