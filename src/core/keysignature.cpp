/* 
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
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
		- Hypophrygian
*/

/*! \deprecated New CanorusML is XML based. It uses keySignatureTypeFromString() and others and the
	default constructor
*/
CAKeySignature::CAKeySignature(QString sPitch, QString sGender, CAStaff *staff, int timeStart)
 : CAMusElement(staff, timeStart) {
 	_musElementType = CAMusElement::KeySignature;
	CAMajorMinorGender gender;
	if (sGender.toUpper()=="MAJOR")
		gender = Major;
	else if (sGender.toUpper()=="MINOR")
		gender = Minor;
	else
		gender = (sPitch.at(0).isLower()?Minor:Major);	// if no gender is defined, set the gender according to the pitch capitals
	
	int pitch = sPitch.toUpper()[0].toLatin1() - 'A';

	signed char accs = -1;
	for (int i=5; i!=pitch; i=(i+1)%7)	// calculate number of accidentals for Major key signatures. Start with F-Major (1 flat) and add 2 sharps mod 7 for every next level
		accs = (accs+2)%7;	
	
	if (gender == Minor)
		accs -= 3;	// Major->Minor transformation: Every same-name minor key sig has -3 accidentals of the major key signature
	
	while (sPitch.size()>1) {
		if (sPitch.toUpper().endsWith("IS"))
			accs += 7;
		else if (sPitch.toUpper().endsWith("ES"))
			accs -= 7;
		else if (sPitch.toUpper().endsWith("AS"))
			accs -= 7;
		
		sPitch.chop(2);	// shorten the end of the pitch for 2 chars
	}
	
	setKeySignatureType(MajorMinor, accs, gender);
}

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

/*!
	Returns the key signature pitch.
	If the scale gender is major, the pitch is returned in upper-case, otherwise in lower.
	eg. "E" for E-major (4 sharps), "ces" for ces-minor (7 flats), "B" for B-major (5 sharps).
	
	\deprecated This isn't used by CanorusML anymore. Should be part of LilyPond parser
*/
const QString CAKeySignature::pitchML() {
	QString ret;
	
	int i;
	int idx=0;
	
	signed char accs = numberOfAccidentals();
	for (i=3; i<accs; i++) {	//start with A and go upwards
		idx = (idx+4)%7;
	}
	for (i=3; i>accs; i--) {	//start with A and go downwards
		idx = (idx+3)%7;
	}
	
	if (_majorMinorGender == Minor) {
		idx = (idx+5)%7;
		ret = ('a' + idx);
		if (accs>2)
			ret = "is";
		else if (accs==-6)
			ret = "s";	//es
		else if (accs<-4)
			ret = "es";
	} else {
		ret = ('A' + idx);
		if (accs>5)
			ret += "is";
		else if (accs==-3)
			ret += "s";	//Es
		else if (accs<-3)
			ret += "es";	
	}
	
	return ret;
}

/*!
	Returns the key signature gender "major" or "minor".
	
	\deprecated Should be moved to LilyPond parser. Use majorMinorGenderToString() instead
*/
const QString CAKeySignature::majorMinorGenderML() {
	switch (_majorMinorGender) {
		case Major: return QString("major");
		case Minor: return QString("minor");
	}
	
	return QString();
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
	if (modus=="locrian") return Locrian;
	if (modus=="hypodorian") return Hypodorian; else
	if (modus=="hypolydian") return Hypolydian; else
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
