/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef INTERVAL_H_
#define INTERVAL_H_

#include <QString>

class CADiatonicPitch;

class CAInterval {
public:
	enum CAQuality {
		Major = 1,
		Minor = -1,
		Perfect = 0,
		Augmented = 2,
		Diminished = -2
	};

	enum CAQuantity {
		Undefined = 0,
		Prime = 1,
		Second = 2,
		Third = 3,
		Fourth = 4,
		Fifth = 5,
		Sixth = 6,
		Seventh = 7,
		Octave = 8
	};

	CAInterval();
	CAInterval( int qlt, int qnt );
	CAInterval( CADiatonicPitch note1, CADiatonicPitch note2, bool absolute=true );

	CAInterval operator~();
	CAInterval operator+( CAInterval );
	CAInterval operator-(CAInterval i) {
		return operator+( CAInterval( i.quality(), i.quantity()*(-1) ) );
	}

	CAInterval operator*( int numerator ) {
		CAInterval interval = *this;
		while ( --numerator ) {
			interval = interval + (*this);
		}

		return interval;
	}

	bool operator==( CAInterval i ) {
		return i.quality()==_qlt && i.quantity()==_qnt;
	}
	bool operator!=( CAInterval i ) {
		return !(operator==(i));
	}

	inline int quality() { return _qlt; }
	inline int quantity() { return _qnt; }

	inline void setQuality( const int qlt ) { _qlt = qlt; }
	inline void setQuantity( const int qnt ) { _qnt = qnt; }

	int semitones();

	static CAInterval fromSemitones( int semitones );
	static const QString qualityToReadable( int k );
	static const QString quantityToReadable( int k );

private:
	int _qlt;
	int _qnt;
};
#endif /* INTERVAL_H_ */
