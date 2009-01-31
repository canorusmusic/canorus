/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef PLAYABLELENGTH_H_
#define PLAYABLELENGTH_H_

#include <QString>

class CABarline;
class CATimeSignature;

class CAPlayableLength {
public:
	enum CAMusicLength {
		Undefined = -1,
		Breve = 0,
		Whole = 1,
		Half = 2,
		Quarter = 4,
		Eighth = 8,
		Sixteenth = 16,
		ThirtySecond = 32,
		SixtyFourth = 64,
		HundredTwentyEighth = 128
	};

	CAPlayableLength();
	CAPlayableLength( CAMusicLength l, int dotted=0 );

	inline const CAMusicLength musicLength() { return _musicLength; }
	inline const int dotted() { return _dotted; }

	inline void setMusicLength( const CAMusicLength l ) { _musicLength = l; }
	inline void setDotted( const int d ) { _dotted = d; }

	bool operator==(CAPlayableLength);
	bool operator!=(CAPlayableLength);

	static const QString musicLengthToString( CAMusicLength length );
	static CAMusicLength musicLengthFromString( const QString length );

	static const int playableLengthToTimeLength( CAPlayableLength length );
	inline static const int musicLengthToTimeLength( CAMusicLength l ) {
		return playableLengthToTimeLength( CAPlayableLength(l) );
	}
	static QList<CAPlayableLength> timeLengthToPlayableLengthList( int timeLength, bool longNotesFirst = true, int dotsLimit = 4 );
	static QList<CAPlayableLength> matchToBars( CAPlayableLength len, int timeStart, CABarline *lastBarline, CATimeSignature *ts, int dotsLimit = 4 );

private:
	CAMusicLength _musicLength; // note, rest length (half, whole, quarter)
	int _dotted;                // number of dots
};
#endif /* PLAYABLELENGTH_H_ */
