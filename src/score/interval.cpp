/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QtGlobal>
#include <QObject>
#include "score/interval.h"
#include "score/diatonicpitch.h"

/*!
	\class CAInterval
	\brief Music interval expressed with diatonical quality and quantity

	This class represent an interval between two notes.
	eg. major second, minor sixth, perfect prime, perfect fifth, augmented fifth etc.

	It consists of two properties:
	- quality (major, minor, perfect, augmented, diminished)
	- quantity (prime, second, third, fourth etc.)

	\note Quantity can also be negative for intervals down.

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
	Constructs a new undefined interval.

	Call setQuality() and setQuantity() to set its properties.
 */
CAInterval::CAInterval() {
	setQuality( 0 );
	setQuantity( 0 );
}

/*!
	Constructs an interval between the pitches \a pitch1 and \a pitch2.
	Interval quantity is positive regardless of the order of pitches, if \a absolute is True (default).
	Otherwise, quantity is negative, if second pitch is lower than the first one.
*/
CAInterval::CAInterval( CADiatonicPitch pitch1, CADiatonicPitch pitch2, bool absolute ) {
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

	if (!absolute && pitch1.noteName()>pitch2.noteName()) {
		setQuantity( -quantity() );
	}
}

/*!
	Returns the inverse of the interval. The reversed interval is always positive.

	eg. seventh -> second, major -> minor and vice versa
*/
CAInterval CAInterval::operator~() {
	int qlt = quality()*(-1);
	int qnt = 8 - ((qAbs(quantity())-2) % 7 + 1);

	return CAInterval( qlt, qnt );
}

/*!
	Returns the sum of two intervals.

	eg. perfect fifth + major third = major seventh
*/
CAInterval CAInterval::operator+( CAInterval i ) {
	CADiatonicPitch p1(0,0);
	CADiatonicPitch p2 = p1 + i + CAInterval( quality(), quantity() );

	return CAInterval( p1, p2 );
}

/*!
	Returns the number of semitones in the interval.

	This is a surjective mapping.
 */
int CAInterval::semitones() {
	int semitones;
	int absQuantity = ((qAbs(quantity())-1) % 7) + 1;

	// major and perfect intervals are default
	switch (absQuantity) {
	case Prime: semitones=0; break;
	case Second: semitones=2; break;
	case Third: semitones=4; break;
	case Fourth: semitones=5; break;
	case Fifth: semitones=7; break;
	case Sixth: semitones=9; break;
	case Seventh: semitones=11; break;
	}

	// minor or diminished/augmented
	switch ( quality() ) {
	case Diminished:
	case Minor:
		semitones -= 1; break;
	case Augmented:
		semitones += 1; break;
	}

	// octaves
	semitones += 12 * (qAbs(quantity())-1) / 7;

	// invert semitones for negative quantity
	if (quantity()<0)
		semitones *= -1;

	return semitones;
}

/*!
	Creates an interval out of the given \a semitones.
	Semitones can also be negative to produce intervals down.

	The interval found in major/minor scales is returned.
	Augmented fourth is used for the tritone.

	This is an injective mapping.
 */
CAInterval CAInterval::fromSemitones( int semitones ) {
	int absSemitones = qAbs(semitones) % 12;
	CAInterval interval;

	switch (absSemitones) {
	case 0:  interval = CAInterval( Perfect, Prime ); break;
	case 1:  interval = CAInterval( Minor, Second ); break;
	case 2:  interval = CAInterval( Major, Second ); break;
	case 3:  interval = CAInterval( Minor, Third ); break;
	case 4:  interval = CAInterval( Major, Third ); break;
	case 5:  interval = CAInterval( Perfect, Fourth ); break;
	case 6:  interval = CAInterval( Augmented, Fourth ); break;
	case 7:  interval = CAInterval( Perfect, Fifth ); break;
	case 8:  interval = CAInterval( Minor, Sixth); break;
	case 9:  interval = CAInterval( Major, Sixth ); break;
	case 10: interval = CAInterval( Minor, Seventh ); break;
	case 11: interval = CAInterval( Major, Seventh ); break;
	}

	// rise the interval for the n-octaves
	interval.setQuantity( interval.quantity() + (qAbs(semitones) / 12) * 7 );

	// set negative interval quantity for negative semitones
	if ( semitones<0 ) {
		interval.setQuantity( interval.quantity()*(-1) );
	}

	return interval;
}

const QString CAInterval::qualityToReadable( int k ) {
	switch (k) {
	case 0: return QObject::tr( "Perfect", "interval" );
	case 1: return QObject::tr( "Major", "interval" );
	case -1: return QObject::tr( "Minor", "interval" );
	case 2: return QObject::tr( "Augmented", "interval" );
	case -2: return QObject::tr( "Diminished", "interval" );
	default: QString::number(k);
	}
}

const QString CAInterval::quantityToReadable( int k ) {
	switch (qAbs(k)) {
	case 1: return QObject::tr( "Prime", "interval" );
	case 2: return QObject::tr( "Second", "interval" );
	case 3: return QObject::tr( "Third", "interval" );
	case 4: return QObject::tr( "Fourth", "interval" );
	case 5: return QObject::tr( "Fifth", "interval" );
	case 6: return QObject::tr( "Sixth", "interval" );
	case 7: return QObject::tr( "Seventh", "interval" );
	case 8: return QObject::tr( "Octave", "interval" );
	case 9: return QObject::tr( "Nineth", "interval" );
	case 10: return QObject::tr( "Tenth", "interval" );
	default: return QString::number(k);
	}
}
