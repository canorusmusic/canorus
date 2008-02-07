/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/diatonicpitch.h"

CADiatonicPitch::CADiatonicPitch() {
	setNoteName( Undefined );
	setAccs( 0 );
}

CADiatonicPitch::CADiatonicPitch( const int& noteName, const int& accs ) {
	setNoteName( noteName );
	setAccs( accs );
}

bool CADiatonicPitch::operator==(CADiatonicPitch p) {
	if ( noteName()==p.noteName() && accs()==p.accs() )
		return true;
	else
		return false;
}

bool CADiatonicPitch::operator==(int p) {
	if ( noteName()==p )
		return true;
	else
		return false;
}
