/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/playablelength.h"

/*!
	\class CAPlayableLength
	\brief Musical length of notes and rests
	
	This class represents a musical length (quarter, half, whole etc.) of notes
	and rests with number of dots.
	
	It consists of two properties:
	- music length (quarter, half, whole etc.)
	- number of dots
	
	Playable length can be easily converted to timeLength. Vice-versa is a bit more
	difficult.
	
	\sa playableLengthToTimeLength(), CARest::composeRests()
*/

CAPlayableLength::CAPlayableLength() {
	setMusicLength( Undefined );
	setDotted( 0 );
}

CAPlayableLength::CAPlayableLength( CAMusicLength l, int dotted ) {
	setMusicLength( l );
	setDotted( dotted );
}

CAPlayableLength::CAMusicLength CAPlayableLength::musicLengthFromString(const QString length) {
	if (length=="undefined") {
		return Undefined;
	} else
	if (length=="breve") {
		return Breve;
	} else
	if (length=="whole") {
		return Whole;
	} else
	if (length=="half") {
		return Half;
	} else
	if (length=="quarter") {
		return Quarter;
	} else
	if (length=="eighth") {
		return Eighth;
	} else
	if (length=="sixteenth") {
		return Sixteenth;
	} else
	if (length=="thirty-second") {
		return ThirtySecond;
	} else
	if (length=="sixty-fourth") {
		return SixtyFourth;
	} else
	if (length=="hundred-twenty-eighth") {
		return HundredTwentyEighth;
	} else
		return Undefined;
}

const QString CAPlayableLength::musicLengthToString(CAPlayableLength::CAMusicLength length) {
	switch ( length ) {
		case Undefined:
			return "undefined";
		case Breve:
			return "breve";
		case Whole:
			return "whole";
		case Half:
			return "half";
		case Quarter:
			return "quarter";
		case Eighth:
			return "eighth";
		case Sixteenth:
			return "sixteenth";
		case ThirtySecond:
			return "thirty-second";
		case SixtyFourth:
			return "sixty-fourth";
		case HundredTwentyEighth:
			return "hundred-twenty-eighth";
		default:
			return "";
	}
}

/*!
	Converts internal enum playableLength to actual timeLength.
	
	\sa CARest::composeRests()
*/
const int CAPlayableLength::playableLengthToTimeLength( CAPlayableLength length ) {
	int timeLength;
	
	switch ( length.musicLength() ) {
		case HundredTwentyEighth:
			timeLength = 8;
			break;
		case SixtyFourth:
			timeLength = 16;
			break;
		case ThirtySecond:
			timeLength = 32;
			break;
		case Sixteenth:
			timeLength = 64;
			break;
		case Eighth:
			timeLength = 128;
			break;
		case Quarter:
			timeLength = 256;
			break;
		case Half:
			timeLength = 512;
			break;
		case Whole:
			timeLength = 1024;
			break;
		case Breve:
			timeLength = 2048;
			break;
		default:            // This should never occur!
			timeLength = 0;
			break;
	}
	
	float factor = 1.0, delta=0.5;
	for (int i=0; i<length.dotted(); i++, factor+=delta, delta/=2); // calculate the length factor out of number of dots
	timeLength = qRound(timeLength*factor);                         // increase the time length for the factor
	
	return timeLength;
}

/*!
	Compares two playable lengths.
*/
bool CAPlayableLength::operator==( CAPlayableLength l ) {
	if ( musicLength()==l.musicLength() && dotted()==l.dotted() )
		return true;
	else
		return false;
}

/*!
	Compares two playable lengths.
*/
bool CAPlayableLength::operator!=( CAPlayableLength l ) {
	return (!operator==(l));
}
