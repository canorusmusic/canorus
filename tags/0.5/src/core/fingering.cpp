/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/fingering.h"
#include "core/note.h"

/*!
	\class CAFingering
	\brief Finger marks
	
	This class represents fingering beside or above/below the note.
	Fingering can include a single finger number or multiple fingers (change fingering
	on one note).
	
	The finger numbers can be written with italic style (original author's fingering when
	making an arrangment) or regular. This property is stored in original().
*/

CAFingering::CAFingering( CAFingerNumber finger, CANote *n, bool original )
 : CAMark( CAMark::Fingering, n ) {
	addFinger( finger );
	setOriginal( original);
	setCommon( false );
}

CAFingering::CAFingering( QList<CAFingerNumber> fingers, CANote *n, bool original )
 : CAMark( CAMark::Fingering, n ) {
	_fingerList = fingers;
	setOriginal( original );
	setCommon( false );
}

CAFingering::~CAFingering() {
}

CAMusElement *CAFingering::clone() {
	return new CAFingering( fingerList(), static_cast<CANote*>(associatedElement()), isOriginal() );
}

int CAFingering::compare( CAMusElement *elt ) {
	if (elt->musElementType()!=CAMusElement::Mark)
		return -2;
	else if (static_cast<CAMark*>(elt)->markType()!=CAMark::Fingering)
		return -1;
	
	int differ=0;
	CAFingering *f = static_cast<CAFingering*>(elt);
	for (int i=0; i<f->fingerList().size() || i<fingerList().size(); i++) {
		if ( i>=f->fingerList().size() || i>=fingerList().size() || f->fingerList().at(i)!=fingerList().at(i) )
			differ++;
	}
	if (static_cast<CAFingering*>(elt)->isOriginal()!=isOriginal())
		differ++;
	
	return differ;
}

const QString CAFingering::fingerNumberToString( CAFingerNumber n ) {
	switch (n) {
	case First:
		return "First";
	case Second:
		return "Second";
	case Third:
		return "Third";
	case Fourth:
		return "Fourth";
	case Fifth:
		return "Fifth";
	case Thumb:
		return "Thumb";
	case LHeel:
		return "LHeel";
	case RHeel:
		return "RHeel";
	case LToe:
		return "LToe";
	case RToe:
		return "RToe";
	case Undefined:
		return "Undefined";
	}
}

CAFingering::CAFingerNumber CAFingering::fingerNumberFromString( const QString f ) {
	if (f=="First") {
		return First;
	} else
	if (f=="Second") {
		return Second;
	} else
	if (f=="Third") {
		return Third;
	} else
	if (f=="Fourth") {
		return Fourth;
	} else
	if (f=="Fifth") {
		return Fifth;
	} else
	if (f=="Thumb") {
		return Thumb;
	} else
	if (f=="LHeel") {
		return LHeel;
	} else
	if (f=="RHeel") {
		return RHeel;
	} else
	if (f=="LToe") {
		return LToe;
	} else
	if (f=="RToe") {
		return RToe;
	} else
		return Undefined;
}
