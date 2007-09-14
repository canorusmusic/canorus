/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
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
	Creates a new predefined clef with type \a type, parent \a staff and start time \a time.
	_timeLength is set to 0.
	\a offset can be custom 0, +7 or -7 etc. and means the 8 written above or below the clef which raises or
	lowers the clef for one octave or whichever interval.
	
	\sa CAPredefinedClefType, CAMusElement
*/
CAClef::CAClef( CAPredefinedClefType type, CAStaff *staff, int time, int offset ) : CAMusElement( staff, time ) {
	_musElementType = CAMusElement::Clef;
	
	setOffset(offset);
	setPredefinedType(type);
}

/*!
	Creates a new clef with type \a type, location of middle C \a c1, time start \a time and parent \a staff.
	Offset does not affect c1. 
	
	\sa CAPredefinedClefType, CAMusElement
*/
CAClef::CAClef( CAClefType type, int c1, CAStaff *staff, int time, int offset ) : CAMusElement( staff, time ) {
	_musElementType = CAMusElement::Clef;
	
	_c1 = c1;
	setOffset(offset);
	setClefType(type);
}

void CAClef::setPredefinedType( CAPredefinedClefType type, int octave ) {
	switch (type) {
		case Treble:
			setClefType(G);
			_c1 = -2 + offset();			
			break;
		case Bass:
			setClefType(F);
			_c1 = 10 + offset();
			break;
		case French:
			setClefType(G);
			_c1 = -4 + offset();
			break;
		case Soprano:
			setClefType(C);
			_c1 = 0 + offset();
			break;
		case Mezzosoprano:
			setClefType(C);
			_c1 = 2 + offset();
			break;
		case Alto:
			setClefType(C);
			_c1 = 4 + offset();
			break;
		case Tenor:
			setClefType(C);
			_c1 = 6 + offset();
			break;
		case Baritone:
			setClefType(C);
			_c1 = 8 + offset();
			break;
		case Varbaritone:
			setClefType(F);
			_c1 = 8 + offset();
			break;
		case Subbass:
			setClefType(F);
			_c1 = 12 + offset();
			break;
	}	
}

/*!
	Sets the clef type to \a type and update _c1 and _centerPitch.
	
	\sa CAClefType, _clefType
*/
void CAClef::setClefType(CAClefType type) {
	_clefType = type;
	
	switch (type) {
	case G: _centerPitch=32; break;
	case F: _centerPitch=24; break;
	case C: _centerPitch=28; break;
	}
	
	_centerPitch += offset();
}

CAClef* CAClef::clone() {
	return new CAClef( _clefType, _c1, static_cast<CAStaff*>(_context), _timeStart, _offset);
}

int CAClef::compare(CAMusElement *elt) {
	if (elt->musElementType()!=CAMusElement::Clef)
		return -1;
	
	int diffs=0;
	if ( _clefType!=static_cast<CAClef*>(elt)->clefType()) diffs++;
	if ( _offset!=static_cast<CAClef*>(elt)->offset()) diffs++;
	if ( _c1!=static_cast<CAClef*>(elt)->c1()) diffs++;
	
	return diffs;
}

/*!
	Converts clef \a type to QString.
	
	\sa CAClefType, clefTypeFromString()
*/
const QString CAClef::clefTypeToString(CAClefType type) {
	switch (type) {
		case G: return "G";
		case F: return "F";
		case C: return "C";
		case PercussionHigh: return "percussion-high";
		case PercussionLow: return "percussion-low";
		case Tab: return "tab";
		default: return "";
	}
}

/*!
	Converts QString \a type to clef type.
	
	\sa CAClefType, clefTypeToString()
*/
CAClef::CAClefType CAClef::clefTypeFromString(const QString type) {
	if (type=="G") return G; else
	if (type=="F") return F; else
	if (type=="C") return C; else
	if (type=="percussion-high") return PercussionHigh; else
	if (type=="percussion-low") return PercussionLow; else
	if (type=="tab") return Tab;
	else return G;
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
