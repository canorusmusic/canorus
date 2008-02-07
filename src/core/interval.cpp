/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QtGlobal>
#include "core/interval.h"
#include "core/diatonicpitch.h"

/*!
	\class CAInterval
	\brief Music interval
	
	This class represent an interval between two notes.
	eg. major second, minor sixth, perfect prime, perfect fifth, augmented fifth etc.
	
	It consists of two properties:
	- quality (major, minor, perfect, augmented, diminished)
	- quantity (prime, second, third, fourth etc.)
	
	\note Quantity can be negative for intervals down.
	
	Intervals can be compared with each other, summed with other intervals or diatonic
	pitches and inverted.
	
	\sa CADiatonicPitch, operator~()
*/

/*!
	Constructs an interval with quality \a qlt and quantity \a qnt.
*/
CAInterval::CAInterval( int qlt, int qnt ) {
	setQuality( qlt );
	setQuantity( qnt );
}

/*!
	Constructs an interval between the pitches \a pitch1 and \a pitch2.
	Interval quantity is absolute regardless of the order of pitches.
*/
CAInterval::CAInterval( CADiatonicPitch pitch1, CADiatonicPitch pitch2 ) {
	CADiatonicPitch pLow, pHigh;
	if ( pitch1.noteName() < pitch2.noteName() ||
	     pitch1.noteName() == pitch2.noteName() && pitch1.accs() <= pitch2.accs() ) {
		pLow = pitch1; pHigh = pitch2;
	} else {
		pLow = pitch2; pHigh = pitch1;
	}
	
	setQuantity( pHigh.noteName() - pLow.noteName() + 1);
	int relQnt = ((quantity()-1)%7)+1;
	int relPLow = pLow.noteName()%7, relPHigh = pHigh.noteName()%7;
	
	int deltaQlt;
	switch (relQnt) {
	case 1: // prime
		deltaQlt = 0;
		break;
	case 2: // second
		if ( relPLow==2 || relPLow==6 )
			deltaQlt = -1;
		else
			deltaQlt = 1;
		break;
	case 3: // third
		if ( relPLow==0 || relPLow==3 || relPLow==4)
			deltaQlt = 1;
		else
			deltaQlt = -1;
		break;
	case 4: // fourth
		if ( relPLow==3 )
			deltaQlt = 2;
		else
			deltaQlt = 0;
		break;
	case 5: // fifth
		if ( relPLow==6 )
			deltaQlt = -2;
		else
			deltaQlt = 0;
		break;
	case 6: // sixth
		if ( relPLow==2 || relPLow==5 || relPLow==6 )
			deltaQlt = -1;
		else
			deltaQlt = 1;
		break;
	case 7: // seventh
		if ( relPLow==0 || relPLow==3 )
			deltaQlt = 1;
		else
			deltaQlt = -1;
		break;
	}
	
	// prime, fourth, fifth are perfect, diminished or augmented
	if ( relQnt==1 || relQnt==4 || relQnt==5 ) {
		if ( deltaQlt==2 && pHigh.accs()-pLow.accs()==-1 ||
		     deltaQlt==0 && pHigh.accs()-pLow.accs()<=-1 ) {
			setQuality( deltaQlt + pHigh.accs() - pLow.accs() - 1 );
		} else
		if ( deltaQlt==2 && pHigh.accs() - pLow.accs() < -1 ) {
			setQuality( deltaQlt + pHigh.accs() - pLow.accs() - 2 );
		} else
		if ( deltaQlt==-2 && pHigh.accs()-pLow.accs()==1 ||
		     deltaQlt==0 && pHigh.accs()-pLow.accs()>=1 ) {
			setQuality( deltaQlt + pHigh.accs() - pLow.accs() + 1 );
		} else
		if ( deltaQlt==-2 && pHigh.accs() - pLow.accs() > 1 ) {
		    setQuality( deltaQlt + pHigh.accs() - pLow.accs() + 2 );
		} else {
			setQuality( deltaQlt + pHigh.accs() - pLow.accs() );
		}
	} else
	// second, third, sixth and seventh cannot be perfect
	if ( deltaQlt==1 && pHigh.accs() - pLow.accs() < 0 ) {
		setQuality( deltaQlt + pHigh.accs() - pLow.accs() - 1 );
	} else
	if ( deltaQlt==-1 && pHigh.accs() - pLow.accs() > 0 ) {
		setQuality( deltaQlt + pHigh.accs() - pLow.accs() + 1 );
	} else {
		setQuality( deltaQlt + pHigh.accs() - pLow.accs() );
	}
}

/*!
	Returns the inverse of the interval.
*/
CAInterval CAInterval::operator~() {
	int qlt = quality()*(-1);
	int qnt = 8 - ((qAbs(quantity())-1) % 7);
	
	return CAInterval( qlt, qnt );
}

/*!
	Returns the sum of two intervals.
*/
CAInterval CAInterval::operator+( CAInterval i ) {
	CADiatonicPitch p1(0,0);
	CADiatonicPitch p2 = p1 + i + CAInterval( quality(), quantity() );
	
	return CAInterval( p1, p2 );
}
