/*!
	Copyright (c) 2019, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/chordname.h"
#include "score/chordnamecontext.h"

/*!
	\class CAChordName
	\brief Chord name

	Chord name (e.g. C, F#m, Gsus4 etc.) inside the CAChordNameContext.
*/

CAChordName::CAChordName( CADiatonicPitch pitch, QString qualityModifier, CAChordNameContext* c, int timeStart, int timeLength )
: CAMusElement ( c, timeStart, timeLength ) {
	setMusElementType(ChordName);
}

CAChordName::~CAChordName() {
}

CAChordName* CAChordName::clone(CAContext* context) {
	if (context && context->contextType() != CAContext::ChordNameContext) {
		return 0;
	}

	return new CAChordName(
	  diatonicPitch(),
	  qualityModifier(),
	  static_cast<CAChordNameContext*>(context),
	  timeStart(),
	  timeLength()
	);
}

int CAChordName::compare(CAMusElement *elt) {
	if (elt->musElementType()!=CAMusElement::ChordName)
		return -2;

	if (static_cast<CAChordName*>(elt)->diatonicPitch()!=diatonicPitch())
		return 1;

	if (static_cast<CAChordName*>(elt)->qualityModifier()!=qualityModifier())
		return 2;

	return 0;
}
