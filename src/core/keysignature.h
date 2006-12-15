/** @file keysignature.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef KEYSIGNATURE_H_
#define KEYSIGNATURE_H_

#include <QString>

#include "core/muselement.h"

class CAStaff;

/**
 * CAKeySignature class represents a simple key signature sign.
 * It also includes other logical information about the scale, for eg. Major/Minor, modes 
 */
class CAKeySignature : public CAMusElement {
	public:
		enum CAKeySignatureType {
			MajorMinor,	// Marks the 7-level Major/Minor
			Modus,
			Custom
		};
		
		enum CAMajorMinorGender {	/// The lower tetrachord of the scale - scale gender
			Major,
			Minor
		};
		
		enum CAMajorMinorShape {	/// The upper tetrachord of the scale - scale shape
			Natural,
			Harmonic,
			Melodic
		};
		
		enum CAModus {	/// Modus type
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
		CAKeySignature(CAKeySignatureType type, signed char accs, CAMajorMinorGender gender, CAStaff *staff, int timeStart);
		CAKeySignature *clone();
		
		/**
		 * This is an overloaded member function provided for convenience.
		 * 
		 * Constructor using the ML arguments.
		 * 
		 * @param pitch The pitch of the key signature. eg. E for E-Major
		 * @param gender Gender of the key signature.
		 */
		/** OBSOLETE */
		CAKeySignature(QString pitch, QString gender, CAStaff *staff, int timeStart);
		
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
		
		/**
		 * Return the key signature pitch.
		 * If the scale gender is major, the pitch is returned in upper-case, otherwise in lower.
		 * eg. "E" for E-major (4 sharps), "ces" for ces-minor (7 flats), "B" for B-major (5 sharps).
		 * 
		 * @return Key signature's scale start pitch.
		 */
		/** OBSOLETE */
		const QString pitchML();
		
		/**
		 * Return the key signature gender major or minor.
		 * 
		 * @return "major" or "minor" if the scale gender is major or minor.
		 */
		/** OBSOLETE */
		const QString majorMinorGenderML();
		
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
		signed char *accidentals() { return _accidentals; }
		
		int compare(CAMusElement* elt);
		
		static const QString keySignatureTypeToString(CAKeySignatureType);
		static CAKeySignatureType keySignatureTypeFromString(const QString);
		static const QString majorMinorGenderToString(CAMajorMinorGender);
		static CAMajorMinorGender majorMinorGenderFromString(const QString);
		static const QString modusToString(CAModus);
		static CAModus modusFromString(const QString);
		/// TODO: MajorMinorShapeTo/FromString

	private:
		CAKeySignatureType _keySignatureType;
		CAModus _modus;
		CAMajorMinorGender _majorMinorGender;
		CAMajorMinorShape _majorMinorShape;
		
		signed char _accidentals[7];	/// Index numbers: [0..6] - C, D, E, F ... B with values: 0 - none, -1 - flat, +1 - sharp
};

#endif /*KEYSIGNATURE_H_*/
