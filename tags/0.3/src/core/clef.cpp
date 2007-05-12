/*
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 *
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#include "core/clef.h"
#include "core/staff.h"

/*!
	\class CAClef
	Represents a clef in the score
	
	This class represents every clef in the score. It directly inherits the base class
	CAMusElement. Element is not playable (_timeLength=0).
*/

/*!
	Creates a new clef with type \a type, parent \a staff and start time \a time.
	_timeLength is set to 0.
	
	\sa CAClefType, CAMusElement
*/
CAClef::CAClef(CAClefType type, CAStaff *staff, int time) : CAMusElement(staff, time) {
	_musElementType = CAMusElement::Clef;
	setClefType(type);
}

/*!
	Sets the clef type to \a type and update _c1 and _centerPitch.
	
	\sa CAClefType, _clefType
*/
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

int CAClef::compare(CAMusElement *elt) {
	if (elt->musElementType()!=CAMusElement::Clef)
		return -1;
	
	int diffs=0;
	if (_clefType!=((CAClef*)elt)->clefType()) diffs++;
	
	return diffs;
}

/*!
	Converts clef \a type to QString.
	
	\sa CAClefType, clefTypeFromString()
*/
const QString CAClef::clefTypeToString(CAClefType type) {
	switch (type) {
		case Treble: return "treble";
		case Bass: return "bass";
		case Alto: return "alto";
		case Tenor: return "tenor";
		case Soprano: return "soprano";
		case PercussionHigh: return "percussion-high";
		case PercussionLow: return "percussion-low";
		default: return "";
	}
}

/*!
	Converts QString \a type to clef type.
	
	\sa CAClefType, clefTypeToString()
*/
CAClef::CAClefType CAClef::clefTypeFromString(const QString type) {
	if (type=="treble") return Treble; else
	if (type=="bass") return Bass; else
	if (type=="alto") return Alto; else
	if (type=="tenor") return Tenor; else
	if (type=="soprano") return Soprano; else
	if (type=="percussion-high") return PercussionHigh; else
	if (type=="percussion-low") return PercussionLow;
	else return Treble;
}

/*!
	\var CAClef::_c1
	Location of the middle C:
		- 0 - 1st line
		- 1 - 1st space
		- -2 - 1st ledger line below staff (ie. C1 in treble clef) etc.
	
	\sa c1(), _centerPitch
*/

/*!
	\var CAClef::_centerPitch
	Location of the clefs physical center:
		- 24 - f for bass clef
		- 32 - g for treble clef
		- 28 - middle C for c-clefs etc.
	This is needed for Y position in the staff calculation.
	
	\sa centerPitch(), _c1
*/
