/* 
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#ifndef KEYSIGNATURE_H_
#define KEYSIGNATURE_H_

#include <QString>

#include "core/muselement.h"

class CAStaff;

class CAKeySignature : public CAMusElement {
public:
	enum CAKeySignatureType {
		MajorMinor, // Marks the standard 7-level Major/Minor
		Modus,
		Custom
	};
	
	enum CAMajorMinorGender {
		Undefined=-1,
		Major,
		Minor
	};
	
	enum CAMajorMinorShape {
		Natural,
		Harmonic,
		Melodic
	};
	
	enum CAModus {
		Ionian,
		Dorian,
		Phrygian,
		Lydian,
		Mixolydian,
		Aeolian,
		Locrian,
		Hypodorian,
		Hypolydian,
		Hypomixolydian,
		Hypophrygian
	};
	
	CAKeySignature(CAKeySignatureType type, signed char accs, CAMajorMinorGender gender, CAStaff *staff, int timeStart);
	~CAKeySignature();
	CAKeySignature *clone();
	
	CAKeySignatureType keySignatureType() { return _keySignatureType; }
	void setKeySignatureType(CAKeySignatureType type, signed char accs, CAMajorMinorGender gender);	
	
	//////////////////////////////
	// Diatonic keys properties //
	//////////////////////////////
	CAMajorMinorGender majorMinorGender() { return _majorMinorGender; }
	CAMajorMinorShape majorMinorShape() { return _majorMinorShape; }
	CAModus modus() { return _modus; }
	void setMajorMinorGender(CAMajorMinorGender gender) { _majorMinorGender = gender; }
	void setMajorMinorShape(CAMajorMinorShape shape) { _majorMinorShape = shape; }
	void setModus(CAModus modus) { _modus = modus; }
	
	signed char numberOfAccidentals();
	signed char *accidentals() { return _accidentals; }
	
	int compare(CAMusElement* elt);
	
	static const QString keySignatureTypeToString(CAKeySignatureType);
	static CAKeySignatureType keySignatureTypeFromString(const QString);
	static const QString majorMinorGenderToString(CAMajorMinorGender);
	static CAMajorMinorGender majorMinorGenderFromString(const QString);
	static const QString modusToString(CAModus);
	static CAModus modusFromString(const QString);
	
private:
	CAKeySignatureType _keySignatureType;
	CAModus _modus;
	CAMajorMinorGender _majorMinorGender;
	CAMajorMinorShape _majorMinorShape;
	
	signed char _accidentals[7];	// Accidentals configuration for each level
};
#endif /*KEYSIGNATURE_H_*/
