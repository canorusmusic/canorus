/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef KEYSIGNATURE_H_
#define KEYSIGNATURE_H_

#include <QString>

#include "core/muselement.h"
#include "core/diatonickey.h"
#include "core/staff.h"

class CAContext;

class CAKeySignature : public CAMusElement {
public:
	enum CAKeySignatureType {
		MajorMinor, // Marks the standard 7-level Major/Minor
		Modus,
		Custom
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

	CAKeySignature(CADiatonicKey k, CAStaff *staff, int timeStart);
	CAKeySignature(CAModus m, CAStaff *staff, int timeStart);
	~CAKeySignature();
	CAKeySignature *clone(CAContext* context=0);
	CAStaff *staff() { return static_cast<CAStaff*>(context()); }

	inline CAKeySignatureType keySignatureType() { return _keySignatureType; }
	inline void setKeySignatureType(CAKeySignatureType type) { _keySignatureType = type; }

	CADiatonicKey diatonicKey() { return _diatonicKey; }
	CAModus modus() { return _modus; }

	void setDiatonicKey(CADiatonicKey k) { _diatonicKey = k; updateAccidentals(); }
	void setModus(CAModus modus) { _modus = modus; }

	signed char *accidentals() { return _accidentals; }

	int compare(CAMusElement* elt);

	static const QString keySignatureTypeToString(CAKeySignatureType);
	static CAKeySignatureType keySignatureTypeFromString(const QString);
	static const QString modusToString(CAModus);
	static CAModus modusFromString(const QString);

private:
	void updateAccidentals();

	CAKeySignatureType _keySignatureType;
	CAModus _modus;
	CADiatonicKey _diatonicKey;

	signed char _accidentals[7];	// Accidentals configuration for each level
};
#endif /* KEYSIGNATURE_H_ */
