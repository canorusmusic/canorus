/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/clef.h"
#include "score/staff.h"
#include "score/mark.h"

/*!
	\class CAClef
	Represents a clef in the score

	This class represents every clef in the score. It directly inherits the base class
	CAMusElement. Element is not playable (_timeLength=0).
*/

/*!
	Creates a new predefined clef with type \a type, parent \a staff and start time \a time.
	_timeLength is set to 0.
	\a offsetInterval can be custom -1,0,1 no offset, +8 or -8 etc. mean the 8 written above or below the clef which raises or
	lowers the clef for one octave or whichever interval. Offset is the musical interval, not the internal offset.

	\sa CAPredefinedClefType, CAMusElement
*/
CAClef::CAClef( CAPredefinedClefType type, CAStaff *staff, int time, int offsetInterval ) : CAMusElement( staff, time ) {
	_musElementType = CAMusElement::Clef;

	_offset = CAClef::offsetFromReadable( offsetInterval );
	setPredefinedType(type);
}

/*!
	Creates a new clef with type \a type, location of middle C \a c1, time start \a time and parent \a staff.
	\a offset is the internal clef offset 0, +7, -7 etc. It does not affect c1.

	\sa CAPredefinedClefType, CAMusElement
*/
CAClef::CAClef( CAClefType type, int c1, CAStaff *staff, int time, int offset ) : CAMusElement( staff, time ) {
	_musElementType = CAMusElement::Clef;

	_c1 = c1;
	_offset = offset;
	setClefType(type);
}

void CAClef::setPredefinedType( CAPredefinedClefType type ) {
	switch (type) {
		case Treble:
			setClefType(G);
			_c1 = -2 - offset();
			break;
		case Bass:
			setClefType(F);
			_c1 = 10 - offset();
			break;
		case French:
			setClefType(G);
			_c1 = -4 - offset();
			break;
		case Soprano:
			setClefType(C);
			_c1 = 0 - offset();
			break;
		case Mezzosoprano:
			setClefType(C);
			_c1 = 2 - offset();
			break;
		case Alto:
			setClefType(C);
			_c1 = 4 - offset();
			break;
		case Tenor:
			setClefType(C);
			_c1 = 6 - offset();
			break;
		case Baritone:
			setClefType(C);
			_c1 = 8 - offset();
			break;
		case Varbaritone:
			setClefType(F);
			_c1 = 8 - offset();
			break;
		case Subbass:
			setClefType(F);
			_c1 = 12 - offset();
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

CAClef* CAClef::clone(CAContext* context) {
	CAClef *c = new CAClef( _clefType, _c1, static_cast<CAStaff*>(context), _timeStart, _offset);

	for (int i=0; i<markList().size(); i++) {
		CAMark *m = static_cast<CAMark*>(markList()[i]->clone(c));
		c->addMark( m );
	}

	return c;
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
	Converts the internal clef offset to the musical interval.
	eg. offset +1 means +2 (supper second),
	           +7 means +8 (supper octave),
	           -7 means -8 (sub octave),
	           0 is an exception and stays 0 (instead of prime)

	This method is usually called when displaying the offset of the key (eg. when rendering it).

	\sa offsetFromReadable()
*/
const int CAClef::offsetToReadable( const int offsetInterval ) {
	if ( !offsetInterval )
		return 0;

	return offsetInterval + offsetInterval/qAbs(offsetInterval);
}

/*!
	Converts the musical interval offset to Canorus internal offset.
	This method is usually called where user inputs the offset interval and it needs to be stored.

	\sa offsetToReadable()
*/
const int CAClef::offsetFromReadable( const int offset ) {
	if ( qAbs(offset)==1 || !offset )
		return 0;

	return offset - offset/qAbs(offset);
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
