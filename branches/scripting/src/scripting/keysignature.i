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
			Diatonic,	//marks the 7-level Major/Minor or modus scale
			Custom
		};
		
		enum CADiatonicGenderType {	///the lower tetrachord of the scale - scale gender
			Major,
			Minor
		};
		
		enum CADiatonicShapeType {	///the upper tetrachord of the scale - scale shape
			Natural,
			Harmonic,
			Melodic
		};
		
		enum CAModusType {	///modus type
			Ionian,
			Dorian,
			Phrygian,
			Lydian,
			Mixolydian,
			Aeolian,
			Locrian,
			Hypodorian,
			Hypolydian,
			Hypophrygian
		};

		/**
		 * Default constructor.
		 * 
		 * TODO: Currently, only ordinary diatonic key signatures can be made.
		 * 
		 * @param type Key signature type. See CAKeySignature::CAKeySignatureType.
		 * @param accs Number of accidentals for an ordinary diatonic key signature. 0 - C-Major, -1 - F-Major, +7 - Cis-Major
		 */
		CAKeySignature(CAKeySignatureType type, signed char accs, CADiatonicGenderType gender, CAStaff *staff, int timeStart);
		
		/**
		 * This is an overloaded member function provided for convenience.
		 * 
		 * Constructor using the ML arguments.
		 * 
		 * @param pitch The pitch of the key signature. eg. E for E-Major
		 * @param gender Gender of the key signature.
		 */
		CAKeySignature(QString pitch, QString gender, CAStaff *staff, int timeStart);
		
		~CAKeySignature();
		
		CAKeySignatureType keySignatureType();
		
		CADiatonicGenderType diatonicGender();
		CADiatonicShapeType diatonicShape();
		CAModusType modus();
		
		void setDiatonicGender(CADiatonicGenderType gender);
		void setDiatonicShape(CADiatonicShapeType shape);
		void setModus(CAModusType modus);
		
		/**
		 * Return the key signature pitch.
		 * If the scale gender is major, the pitch is returned in upper-case, otherwise in lower.
		 * eg. "E" for E-major (4 sharps), "ces" for ces-minor (7 flats), "B" for B-major (5 sharps).
		 * 
		 * @return Key signature's scale start pitch.
		 */
		const QString pitchML();
		
		/**
		 * Return the key signature gender major or minor.
		 * 
		 * @return "major" or "minor" if the scale gender is major or minor.
		 */
		const QString diatonicGenderML();
		
		/**
		 * Count all the accidentals and return their sum.
		 * Every sharp counts +1, every flat counts -1.
		 * 
		 * @return Sum of all the accidentals in the scale.
		 */
		signed char numberOfAccidentals();
		
		/**
		 * Return the pointer to the array of accidentals for every level in the scale.
		 * The levels can have the following values:
		 * -  0: natural
		 * - +1: sharp
		 * - +2: double sharp
		 * ...
		 * - -1: flat
		 * - -2: double flat
		 * ...
		 * 
		 * @return Pointer to the array of levels in the scale.
		 */ 
		signed char *accidentals();
};
