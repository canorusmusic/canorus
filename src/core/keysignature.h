/** @file keysignature.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef KEYSIGNATURE_H_
#define KEYSIGNATURE_H_


#include "core/muselement.h"

class CAStaff;

/**
 * CAKeySignature class represents a simple key signature sign.
 * Note that any other logic behind it (major, minor, scale type, moduses etc.) are handled by other classes.
 */
class CAKeySignature : public CAMusElement {
	public:
		enum CAKeySignatureType {
			Diatonic,
			Custom
		};

		/**
		 * Default constructor.
		 * 
		 * TODO: Currently, only ordinary diatonic key signatures can be made.
		 * 
		 * @param type Key signature type. See CAKeySignature::CAKeySignatureType.
		 * @param accs Number of accidentals for an ordinary diatonic key signature. 0 - C-Major, -1 - F-Major, +7 - Cis-Major
		 */
		CAKeySignature(CAKeySignatureType type, signed char accs, CAStaff *staff, int timeStart);
		~CAKeySignature();
		
		CAKeySignatureType keySignatureType() { return _keySignatureType; }
		
		signed char numberOfAccidentals();
		signed char *accidentals() { return _accidentals; }

	private:
		CAKeySignatureType _keySignatureType;
		
		signed char _accidentals[7];	///index numbers: [0..6] - C, D, E, F ... B with values: 0 - none, -1 - flat, +1 - sharp
};

#endif /*KEYSIGNATURE_H_*/
