/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/instrumentchange.h"
#include "core/note.h"

/*!
	\class CAInstrumentChange
	\brief Instrument change during the score
	
	This class is used to allow the playback to change the default voice instrument.
	
	\todo Instrument is now int. This should probably be moved out someday to enum
	      (eg. into CAInstrument?).
*/

CAInstrumentChange::CAInstrumentChange( int instrument, CANote *note )
 : CAMark( CAMark::InstrumentChange, note ) {
	setInstrument(instrument);
}

CAInstrumentChange::~CAInstrumentChange() {
}

CAMusElement* CAInstrumentChange::clone() {
	return new CAInstrumentChange( instrument(), static_cast<CANote*>(associatedElement()) );
}

int CAInstrumentChange::compare( CAMusElement *elt ) {
	if (elt->musElementType()!=CAMusElement::Mark)
		return -2;
	
	if (static_cast<CAMark*>(elt)->markType()!=CAMark::InstrumentChange)
		return -1;
	
	if (static_cast<CAInstrumentChange*>(elt)->instrument()!=instrument())
		return 1;
	
	return 0;
}
