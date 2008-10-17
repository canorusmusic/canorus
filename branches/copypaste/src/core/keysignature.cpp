/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/keysignature.h"
#include "core/staff.h"
#include "core/mark.h"

/*!
	\class CAKeySignature
	\brief Represents a key signature sign in the staff
	
	CAKeySignature represents a key signature sign (static accidentals) in the staff.
	The actual key signature key is stored (depends on type though - currently only major/minor
	diatonic keys are supported) in _diatonicKey.
	
	\sa CADiatonicKey
*/

/*!
	Creates a new key signature of type MajorMinor, diatonic key \a dKey, parent \a staff and \a timeStart.
	
	Number of accidentals is a signed number, positive for sharps, negative for flats.
	eg.
		- 0 - C-Major
		- -1 - F-Major
		- +7 - Cis-Major etc.	
*/
CAKeySignature::CAKeySignature( CADiatonicKey k, CAStaff *staff, int timeStart )
 : CAMusElement(staff, timeStart) {
 	setMusElementType( CAMusElement::KeySignature );
 	
 	setKeySignatureType( MajorMinor );
 	setDiatonicKey(k);
}

/*!
	Creates a new key signature of type Modus, modus type \a m, parent \a staff and \a timeStart.
	
	\todo Modus on different pitches	
*/
CAKeySignature::CAKeySignature(CAModus m, CAStaff *staff, int timeStart)
 : CAMusElement(staff, timeStart) {
 	setMusElementType( CAMusElement::KeySignature );
 	
 	setKeySignatureType( Modus );
 	setModus(m);
}

/*!
	\enum CAKeySignature::CAKeySignatureType
	Type of the key signature:
		- MajorMinor
			Standard diatonic scale found by circle of fifths
		- Modus
			One of the moduses found in middle-age
		- Custom
			Custom scale. Modern scales, harmony of fourths, local scales
*/

/*!
	\enum CAKeySignature::CAModus
	Modus types:
		- Ionian
		- Dorian
		- Phrygian
		- Lydian
		- Mixolydian
		- Aeolian
		- Locrian
		- Hypodorian
		- Hypolydian
		- Hypomixolydian
		- Hypophrygian
*/

/*!
	\todo Implement non major-minor types
*/
void CAKeySignature::updateAccidentals() {
	if (keySignatureType() == MajorMinor) {
		for (int i=0; i<7; i++)	// clean up the _accidentals array
			_accidentals[i] = 0;
		
		// generate the _accidentals array according to the given number of the accidentals
		// eg. _accidentals[3] = -1; means flat on the 3rd note (counting from 0), this means this key signature has Fes instead of F.
		int idx = 3;
		for (int i=0; i<_diatonicKey.numberOfAccs(); i++) { // key signatures with sharps
			_accidentals[idx] = 1;
			idx = (idx+4)%7; // the circle of fifths in positive direction - add a Fifth
		}
		
		idx = 6;
		for (int i=0; i>_diatonicKey.numberOfAccs(); i--) { // key signatures with flats
			_accidentals[idx] = -1;
			idx = (idx+3)%7; // the circle of fifths in negative direction - add a Fourth
		}
	}
}

CAKeySignature::~CAKeySignature() {
}

CAKeySignature* CAKeySignature::clone() {
	CAKeySignature *k = 0;
	
	switch (keySignatureType()) {
	case MajorMinor:
		k = new CAKeySignature( diatonicKey(), static_cast<CAStaff*>(context()), timeStart());
	}
	
	for (int i=0; i<markList().size(); i++) {
		CAMark *m = static_cast<CAMark*>(markList()[i]->clone());
		m->setAssociatedElement(k);
		k->addMark( m );
	}
	
	return k;
}

int CAKeySignature::compare(CAMusElement *elt) {
	if (elt->musElementType()!=CAMusElement::KeySignature)
		return -1;
	
	int diffs=0;
	if ( keySignatureType()!=static_cast<CAKeySignature*>(elt)->keySignatureType()) diffs++;
	else {
		if ( keySignatureType()==MajorMinor )
			if ( diatonicKey()!=static_cast<CAKeySignature*>(elt)->diatonicKey() )
				diffs++;
	}
	
	return diffs;
}

CAKeySignature::CAKeySignatureType CAKeySignature::keySignatureTypeFromString(const QString type) {
	if (type=="major-minor") {
		return MajorMinor;
	} else
	if (type=="modus") {
		return Modus;
	} else
	if (type=="custom") {
		return Custom;
	} else
		return Custom;
}

const QString CAKeySignature::keySignatureTypeToString(const CAKeySignatureType type) {
	switch (type) {
	case MajorMinor:
		return "major-minor";
	case Modus:
		return "modus";
	case Custom:
		return "custom";
	}
}

const QString CAKeySignature::modusToString(CAModus modus) {
	switch (modus) {
		case Ionian: return "ionian";
		case Dorian: return "dorian";
		case Phrygian: return "phrygian";
		case Lydian: return "lydian";
		case Mixolydian: return "mixolydian";
		case Aeolian: return "aeolian";
		case Locrian: return "locrian";
		case Hypodorian: return "hypodorian";
		case Hypolydian: return "hypolydian";
		case Hypomixolydian: return "hypomixolydian";
		case Hypophrygian: return "hypophrygian";
		default: return "";
	}
}

CAKeySignature::CAModus CAKeySignature::modusFromString(const QString modus) {
	if (modus=="ionian") return Ionian; else
	if (modus=="dorian") return Dorian; else
	if (modus=="phrygian") return Phrygian; else
	if (modus=="lydian") return Lydian; else
	if (modus=="mixolydian") return Mixolydian; else
	if (modus=="aeolian") return Aeolian; else
	if (modus=="locrian") return Locrian; else
	if (modus=="hypodorian") return Hypodorian; else
	if (modus=="hypolydian") return Hypolydian; else
	if (modus=="hypomixolydian") return Hypomixolydian; else
	if (modus=="hypophrygian") return Hypophrygian;
	else return Ionian;
}

/*!
	\fn CAKeySignature::accidentals()
	Returns the array of accidentals for every level in the scale.
	
	The levels can have the following values:
		- 0 - natural
		- 1 - sharp
		- 2 - double sharp
		- ...
		- -1 - flat
		- -2 - double flat
		- ...
*/ 

/*!
	\var CAKeySignature::_accidentals
	Accidentals configuration for each level.
	
	Indexes: [0..6] - C, D, E, F ... B
	Values: 0 - none, -1 - flat, +1 - sharp
	
	\sa accidentals(), numberOfAccidentals()
*/
