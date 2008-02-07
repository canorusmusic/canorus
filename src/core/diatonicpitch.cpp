/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/diatonicpitch.h"

/*!
	\class CADiatonicPitch
	\brief Musical note pitch
	
	This is a typical music presentation of the note pitch.
	
	It consists of two properties:
	- note name (C-0, D-1, E-2 etc.)
	- accidentals (0-neutral, 1-one sharp, -1-one flat etc.)
	
	Note name begins with sub-contra C.
	
	Diatonic pitches can be compared with each other or only note names
	and summed with intervals.
	
	\sa CAInterval, operator+(), CAMidiDevice::diatonicPitchToMidi()
*/

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

/*!
	Calculates a new pitch using the old pitch + interval.
*/
CADiatonicPitch CADiatonicPitch::operator+( CAInterval i ) {
	CADiatonicPitch dp;
	
	if ( i.quantity()<0 ) { // inverse interval, if negative
		dp.setNoteName( noteName() + (((i.quantity()+2) / 7) - 1) * 7 ); // lower the pitch for n-octaves
		 i=~i; // inverse interval
	}
	
	dp.setNoteName( noteName()+i.quantity()-1 );
	int deltaAccs=0;
	
	int relP = noteName()%7;
	int relQnt = ((i.quantity()-1) % 7) + 1;
	switch (relQnt) {
	case 1: // prime
		deltaAccs=0;
		break;
	case 2: // second
		if ( relP==2 || relP==6 )
			deltaAccs=1;
		else
			deltaAccs=0;
		break;
	case 3: // third
		if ( relP==0 || relP==3 || relP==4 )
			deltaAccs=0;
		else
			deltaAccs=1;
		break;
	case 4: // fourth
		if ( relP==3 )
			deltaAccs=-1;
		else
			deltaAccs=0;
		break;
	case 5: // fifth
		if ( relP==6 )
			deltaAccs=1;
		else
			deltaAccs=0;
	case 6: // sixth
		if ( relP==2 || relP==5 || relP==6 )
			deltaAccs=1;
		else
			deltaAccs=0;
	case 7: // seventh
		if ( relP==0 || relP==3 )
			deltaAccs=0;
		else
			deltaAccs=1;
	}
	
	if ( relP==4 || relP==5 || relP==1 ) {
		if (i.quality()<0)
			dp.setAccs( deltaAccs + accs() + i.quality() + 1 );
		else if (i.quality()>0)
			dp.setAccs( deltaAccs + accs() + i.quality() - 1 );
		else
			dp.setAccs( deltaAccs + accs() );
	} else {
		if (i.quality()<0)
			dp.setAccs( deltaAccs + accs() + i.quality() );
		else if (i.quality()>0)
			dp.setAccs( deltaAccs + accs() + i.quality() - 1 );
	}
	
	return dp;
}
