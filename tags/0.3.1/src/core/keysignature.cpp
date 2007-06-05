/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/keysignature.h"
#include "core/staff.h"

/*!
	\class CAKeySignature
	\brief Represents a key signature sign in the staff
	
	CAKeySignature represents a key signature sign (prefixed accidentals) in the staff.
	It also includes other logical information about the scale, for eg. Major/Minor or modes
	
	\todo MajorMinorShapeTo/FromString
	\todo Currently, only diatonic key signatures can be made (the ones in circle of fifths).
*/

/*!
	Creates a new key signature of type \a type, number of accidentals \a accs, specified \a gender,
	parent \a staff and \a timeStart.
	
	Number of accidentals is a signed number, positive for sharps, negative for flats.
	eg.
		- 0 - C-Major
		- -1 - F-Major
		- +7 - Cis-Major etc.	
*/
CAKeySignature::CAKeySignature(CAKeySignatureType type, signed char accs, CAMajorMinorGender gender, CAStaff *staff, int timeStart)
 : CAMusElement(staff, timeStart) {
 	_musElementType = CAMusElement::KeySignature;
 	
 	setKeySignatureType(type, accs, gender);
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
	\enum CAKeySignature::CAMajorMinorGender
	The lower tetrachord of the scale - gender:
		- Major
		- Minor
*/

/*!
	\enum CAKeySignature::CAMajorMinorShape
	The upper tetrachord of the scale - shape:
		- Natural
		- Harmonic
		- Melodic
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
void CAKeySignature::setKeySignatureType(CAKeySignatureType type, signed char accs, CAMajorMinorGender gender) {
 	_keySignatureType = type;
 	
 	if (type == MajorMinor) {
		_majorMinorGender = gender;
		
		for (int i=0; i<7; i++)	// clean up the _accidentals array
			_accidentals[i] = 0;
		
		// generate the _accidentals array according to the given number of the accidentals
		// eg. _accidentals[3] = -1; means flat on the 3rd note (counting from 0), this means this key signature has Fes instead of F.
		int idx = 3;
		for (int i=0; i<accs; i++) {	// key signatures with sharps
			_accidentals[idx] = 1;
			idx = (idx+4)%7;	// the circle of fifths in positive direction - add a Fifth
		}
		
		idx = 6;
		for (int i=0; i>accs; i--) {	// key signatures with flats
			_accidentals[idx] = -1;
			idx = (idx+3)%7;	// the circle of fifths in negative direction - add a Fourth
		}
	}
}

CAKeySignature::~CAKeySignature() {
}

CAKeySignature* CAKeySignature::clone() {
	return new CAKeySignature(_keySignatureType, numberOfAccidentals(), _majorMinorGender, (CAStaff*)_context, _timeStart);
}

/*!
	Counts all the accidentals and returns their sum.
	Every sharp counts +1, every flat counts -1.
	
	\sa _accidentals, accidentals()
*/
signed char CAKeySignature::numberOfAccidentals() {
	signed char sum=0;
	for (int i=0; i<7; i++)
		sum += _accidentals[i];
	
	return sum;
}

int CAKeySignature::compare(CAMusElement *elt) {
	if (elt->musElementType()!=CAMusElement::KeySignature)
		return -1;
	
	int diffs=0;
	if (_keySignatureType!=((CAKeySignature*)elt)->keySignatureType()) diffs++;
	else {
		if (_majorMinorGender!=((CAKeySignature*)elt)->majorMinorGender()) diffs++;
		if (numberOfAccidentals()!=((CAKeySignature*)elt)->numberOfAccidentals()) diffs++;
	}
	
	return diffs;
}

const QString CAKeySignature::keySignatureTypeToString(CAKeySignatureType type) {
	switch (type) {
		case MajorMinor: return "major-minor";
		case Modus: return "modus";
		case Custom: return "custom";
		default: return "";
	} 
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

const QString CAKeySignature::majorMinorGenderToString(CAMajorMinorGender gender) {
	switch (gender) {
		case Major: return "major";
		case Minor: return "minor";
		default: return "";
	}
}

CAKeySignature::CAMajorMinorGender CAKeySignature::majorMinorGenderFromString(const QString gender) {
	if (gender=="major") return Major; else
	if (gender=="minor") return Minor;
	else return Major;
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
