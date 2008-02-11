/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/diatonickey.h"

/*!
	\class CADiatonicKey
	\brief Musical key
	
	This is a typical music presentation of the key.
	For example, C-major, d-minor, e-flat-minor etc.
	
	It consists of two properties:
	- diatonic pitch (cis, des etc.)
	- gender (major, minor etc.)
	
	Valid range of diatonic pitch for the key is from C (0) to B (6)
	
	\sa CADiatonicPitch
*/

CADiatonicKey::CADiatonicKey() {
	setDiatonicPitch( CADiatonicPitch( 0 ) );
	setGender( Major );
	setShape( Natural );
}

CADiatonicKey::CADiatonicKey( const QString& key ) {
	setDiatonicPitch(CADiatonicPitch( key ));
	setGender( key[0].isUpper()?Major:Minor );
	if ( gender()==Major )
		setShape( Natural );
	else
		setShape( Harmonic );
}

CADiatonicKey::CADiatonicKey( const int& nAccs, const CAGender& gender ) {
	setGender( gender );
	setShape( Natural );
	
	int pitch = ((4*nAccs) % 7) + ((nAccs < 0) ? 7 : 0);
	
	if (gender==CADiatonicKey::Minor) // find the parallel minor key
		pitch = (pitch + 5) % 7;
	
	signed char accs = 0;
	
	if (nAccs>5 && gender==CADiatonicKey::Major)
		accs = (nAccs-5)/7+1;
	else
	if (nAccs>2 && gender==CADiatonicKey::Minor)
		accs = (nAccs-2)/7 + 1;
	else
	if (nAccs<-1 && gender==CADiatonicKey::Major)
		accs = (nAccs+1)/7-1;
	else
	if (nAccs<-4 && gender==CADiatonicKey::Minor)
		accs = (nAccs+4)/7 - 1;
	
	setDiatonicPitch( CADiatonicPitch(pitch, accs) );
}

CADiatonicKey::CADiatonicKey( const CADiatonicPitch& pitch, const CAGender& gender ) {
	setDiatonicPitch( pitch );
	setGender( gender );
	
	if ( gender==Major )
		setShape( Natural );
	else
		setShape( Harmonic );
}

CADiatonicKey::CADiatonicKey( const CADiatonicPitch& pitch, const CAGender& gender, const CAShape& shape ) {
	setDiatonicPitch( pitch );
	setGender( gender );
	setShape( shape );
}

int CADiatonicKey::numberOfAccs() {
	// calculate accs for minor keys
	int accs = ( ((diatonicPitch().noteName()+2) * 2 + 4) % 7 - 4 );
	
	accs += 7*diatonicPitch().accs();
	
	if (gender()==CADiatonicKey::Major)
		accs += 3;
	
	return accs;
}

bool CADiatonicKey::operator==(CADiatonicKey k) {
	if ( diatonicPitch()==k.diatonicPitch() && gender()==k.gender() )
		return true;
	else
		return false;
}

const QString CADiatonicKey::genderToString( CAGender gender ) {
	switch (gender) {
		case Major: return "major";
		case Minor: return "minor";
		default: return "";
	}
}

CADiatonicKey::CAGender CADiatonicKey::genderFromString( const QString gender ) {
	if (gender=="major") return Major; else
	if (gender=="minor") return Minor;
	else return Major;
}

const QString CADiatonicKey::shapeToString( CAShape shape ) {
	switch (shape) {
		case Natural: return "natural";
		case Harmonic: return "harmonic";
		case Melodic: return "melodic";
		default: return "";
	}
}

CADiatonicKey::CAShape CADiatonicKey::shapeFromString( const QString shape ) {
	if (shape=="natural") return Natural; else
	if (shape=="harmonic") return Harmonic; else
	if (shape=="melodic") return Melodic;
	else return Natural;
}

/*!
	Generates a readable name of the diatonic key.
	eg. -3 accidentals & major => "Es"
	    +6 accidentals & minor => "dis"
*/
const QString CADiatonicKey::diatonicKeyToString( CADiatonicKey k ) {
	// calculate key signature pitch from number of accidentals
	int pitch = ((4*k.numberOfAccs()) % 7) + ((k.numberOfAccs() < 0) ? 7 : 0);
	
	if (k.gender()==CADiatonicKey::Minor) // find the parallel minor key
		pitch = (pitch + 5) % 7;
	
	signed char accs = 0;
	
	if (k.numberOfAccs()>5 && k.gender()==CADiatonicKey::Major)
		accs = (k.numberOfAccs()-5)/7+1;
	else
	if (k.numberOfAccs()>2 && k.gender()==CADiatonicKey::Minor)
		accs = (k.numberOfAccs()-2)/7 + 1;
	else
	if (k.numberOfAccs()<-1 && k.gender()==CADiatonicKey::Major)
		accs = (k.numberOfAccs()+1)/7-1;
	else
	if (k.numberOfAccs()<-4 && k.gender()==CADiatonicKey::Minor)
		accs = (k.numberOfAccs()+4)/7 - 1;
	
	QString name;
	
	name = (char)((pitch+2)%7 + 'a');
	
	for (int i=0; i < accs; i++)
		name += "is";	// append as many -is-es as necessary
	
	for (int i=0; i > accs; i--) {
		if ( (name == "e") || (name == "a") )
			name += "s";	// for pitches E and A, only append single -s the first time
		else if (name[0]=='a')
			name += "as";	// for pitch A, append -as instead of -es
		else
			name += "es";	// otherwise, append normally as many es-es as necessary
	}
	
	if (k.gender()==CADiatonicKey::Major)
		name[0] = name[0].toUpper();
	
	return name;
}

/*!
	\enum CADiatonicKey::CAGender
	The lower tetrachord of the scale - gender:
		- Major
		- Minor
*/

/*!
	\enum CADiatonicKey::CAShape
	The upper tetrachord of the scale - shape:
		- Natural
		- Harmonic
		- Melodic
*/
