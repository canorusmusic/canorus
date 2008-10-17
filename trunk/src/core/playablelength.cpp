/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/playablelength.h"
#include "core/timesignature.h"
#include "core/barline.h"

#include <iostream>

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
	Compute for a given time length the CAPlayableLengths. In the general case
	this could result in several notes to stay within canorus limits.
	In canorus gui we have max. 4 dots.

	To make this computation fast we take in account that note durations are
	a binary representation of the time duration. The breve value is not exactly
	a log2 value, so we do this nonlinear operation through the method of
	computation (see **).

	Limitations: Maximum four dots per note, bar the smallest resulting time duration
	is SixtyFourth;

	Todo: Allow change of limitations as function parameters, which are longest note
	and number of dots.
*/
QList<CAPlayableLength> CAPlayableLength::timeLengthToPlayableLengthList( int t ) {

	QList<CAPlayableLength> pl;
	int workTime = t;	// this is the register that decreases for every item processed
	const int breveTime = playableLengthToTimeLength( Breve );
	int maxDots = 4;

	int leadingBreves = workTime & ~(2*breveTime-1);
	while (leadingBreves>breveTime) {
		pl << CAPlayableLength( Breve );
		leadingBreves -= breveTime;
	}

	// now only a breve with possibly many dots is left
	workTime &= 2*breveTime-1;

	int currentTime = breveTime;
	int logCurrentMusLenPlusOne = 0;
	int dots;
	bool findNote = true;
	while (workTime && (currentTime >= musicLengthToTimeLength( HundredTwentyEighth ))) {
	
		if (findNote) {
			if (workTime & currentTime) {
				// Now we reverse log2 and exponentiate and do the nonlinear mapping of breve (**)
				// when the value 1 is erased by division with 2::
				pl << CAPlayableLength( CAMusicLength( (1<<logCurrentMusLenPlusOne)/2 ));
				dots = maxDots;
				findNote = maxDots > 0 ? false : true;
			} else {
				findNote = true;
			}
		} else {
			// try to find a dot for the current note
			if (workTime & currentTime) {
				pl.back().setDotted( pl.back().dotted() + 1 );
				dots--;
				findNote = dots > 0 ? false : true;
			} else
				findNote = true;
		}
		workTime &= ~currentTime;
		currentTime /= 2;
		logCurrentMusLenPlusOne++;
	}
	return pl;
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

/*!
	Split a playable to match a given bar border and bar length.
*/
QList<CAPlayableLength> CAPlayableLength::matchToBars( CAPlayableLength len, int timeStart, CABarline *lastBarline, CATimeSignature *ts ) {
	QList<CAPlayableLength> unchanged; unchanged << len;
	if (!ts) return unchanged;
	
	int beat = ts->beat();
	switch (beat) {
	case 2:
	case 4:
	case 8:		break;
	default:	return unchanged;
	}
	int barLength = CAPlayableLength::playableLengthToTimeLength(
				CAPlayableLength( static_cast<CAPlayableLength::CAMusicLength>(ts->beat()) ) ) * ts->beats();
	int barRest = ( lastBarline ? lastBarline->timeStart() : 0 ) + barLength - timeStart;
	// no change when bar lengths are bogus
	if (barRest < 0 || barRest > barLength) return unchanged;

	int noteLen = len.playableLengthToTimeLength( len );

	// now we really do a split
	QList<CAPlayableLength> list;
	int tSplit = barRest ? barRest : barLength;
	do {
		if (noteLen) {
			tSplit = tSplit > noteLen ? noteLen : tSplit;
			list << timeLengthToPlayableLengthList( tSplit );
			noteLen -= tSplit;
			tSplit = noteLen > barLength ? barLength : noteLen;
		} else {
			break;
		}
	} while (true);
	
	return list;
}

