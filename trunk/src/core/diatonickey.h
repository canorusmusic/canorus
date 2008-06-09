/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef DIATONICKEY_H_
#define DIATONICKEY_H_

#include "core/diatonicpitch.h"
#include <QString>

class CADiatonicKey {
public:
	enum CAGender {
		Major,
		Minor
	};
	
	enum CAShape {
		Natural,
		Harmonic,
		Melodic
	};
	
	CADiatonicKey();
	CADiatonicKey( const QString& key );
	CADiatonicKey( const int& numberOfAccs, const CAGender& gender );
	CADiatonicKey( const CADiatonicPitch& pitch, const CAGender& gender );
	CADiatonicKey( const CADiatonicPitch& pitch, const CAGender& gender, const CAShape& shape );
	
	bool operator==(CADiatonicKey);
	inline bool operator!=(CADiatonicKey p) { return !operator==(p); }
#ifndef SWIG
	void operator=(const QString& key);
#endif
	inline CADiatonicPitch diatonicPitch() { return _diatonicPitch; }
	inline const CAGender gender() { return _gender; }
	inline const CAShape shape() { return _shape; }
	
	inline void setDiatonicPitch( const CADiatonicPitch p ) { _diatonicPitch = p; }
	inline void setGender( const CAGender g ) { _gender = g; }
	inline void setShape( const CAShape s ) { _shape = s; }
	
	static const QString shapeToString( CAShape );
	static CAShape shapeFromString( const QString );
	static const QString genderToString( CAGender );
	static CAGender genderFromString( const QString );
	
	static const QString diatonicKeyToString( CADiatonicKey k );
	static CADiatonicKey diatonicKeyFromString( const QString );
	
	int numberOfAccs();
	
private:
	CADiatonicPitch _diatonicPitch; // pitch of the key
	CAGender        _gender;        // major, minor
	CAShape         _shape;         // natural, harmonic, melodic
};
#endif /* DIATONICKEY_H_ */
