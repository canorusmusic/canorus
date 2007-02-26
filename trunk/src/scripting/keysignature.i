/** @file scripting/keysignature.i
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

%{
#include "core/keysignature.h"
%}

/**
 * Swig implementation of CAKeySignature.
 */
%rename(KeySignature) CAKeySignature;
class CAKeySignature : public CAMusElement {
	public:
		enum CAKeySignatureType {
			Undefined=-1,
			MajorMinor,
			Modus,
			Custom
		};
		
		enum CAMajorMinorGender {
			Undefined=-1,
			Major,
			Minor
		};
		
		enum CAMajorMinorShape {
			Undefined=-1,
			Natural,
			Harmonic,
			Melodic
		};
		
		enum CAModus {
			Undefined=-1,
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
		CAKeySignature *clone();
		
		~CAKeySignature();
		
		CAKeySignatureType keySignatureType() { return _keySignatureType; }
		void setKeySignatureType(CAKeySignatureType type, signed char accs, CAMajorMinorGender gender);	/// TODO: Non major-minor types
		
		/****************************
		 * Diatonic keys properties *
		 ****************************/
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
};
