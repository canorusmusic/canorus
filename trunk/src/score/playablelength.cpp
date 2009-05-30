/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/playablelength.h"
#include "score/timesignature.h"
#include "score/barline.h"

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
	this could result in several notes. In the canorus GUI we can have max. 4 dots.
	By default longer notes appear first in the list, but with negating longNotesFirst
	the short ones appear first. This is useful for end of bar notes.

	To make this computation fast we take in account that note durations are
	a binary presentation of the time duration. The breve value is not exactly
	a log2 value, so we do this singular nonlinear operation through the method of
	computation (see **).

	Limitations: Maximum four dots per note, and the smallest resulting time duration
	is SixtyFourth;

	Todo: Allow change of limitations as function parameters, which are longest note
	and number of dots.
*/
QList<CAPlayableLength> CAPlayableLength::timeLengthToPlayableLengthList( int t, bool longNotesFirst, int dotsLimit ) {

	QList<CAPlayableLength> pl;
	int workTime = t;	// this is the register that decreases for every item processed
	const int breveTime = playableLengthToTimeLength( Breve );

	int leadingBreves = workTime & ~(2*breveTime-1);
	while (leadingBreves>=breveTime) {
		pl << CAPlayableLength( Breve );
		leadingBreves -= breveTime;
	}

	// Now only maximum one breve with possibly many dots is left
	workTime &= 2*breveTime-1;
	// and as a safety measure we suppress time elements smaller than 128ths.
	workTime &= ~(playableLengthToTimeLength( HundredTwentyEighth )-1);
	if (dotsLimit > 4) dotsLimit = 4;

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
				dots = dotsLimit;
				findNote = dotsLimit > 0 ? false : true;
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
	// If not short notes first, for example at the end of bar, we reverse the list.
	int i,j;
	if (!longNotesFirst) for( i=0, j=pl.size()-1; i<j ; i++,j-- ) pl.swap(i,j);
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
	This function is used at music input with a midi keyboard.
*/
QList<CAPlayableLength> CAPlayableLength::matchToBars( CAPlayableLength len, int timeStart, CABarline *lastBarline, CATimeSignature *ts, int dotsLimit ) {

	// If something is strange or undoable we prepare for returning the length unchanged.
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
	if (!lastBarline || lastBarline->timeStart() < ts->timeStart() || timeStart == ts->timeStart())
		barRest = 0;

	// no change when bar lengths are bogus
	if (barRest < 0 || barRest > barLength) return unchanged;

	int noteLen = len.playableLengthToTimeLength( len );

	// now we really do a split
	QList<CAPlayableLength> list;
	int tSplit = barRest ? barRest : barLength;
	bool longNotesFirst = barRest ? false : true;
	while (noteLen) {
		tSplit = tSplit > noteLen ? noteLen : tSplit;
		list << timeLengthToPlayableLengthList( tSplit, longNotesFirst, dotsLimit );
		noteLen -= tSplit;
		tSplit = noteLen > barLength ? barLength : noteLen;
		longNotesFirst = true;
	}

	return list;
}

/*!
	Split a playable length to match a virtual or real next barline by recognising a given previous barline
	and bar length by a given time signature.
	In lack of barline or timesignature asumptions are made, maybe resulting in returning an empty split list.

	This function is used in midi import, could be used at mouse note input too, probably. See also \a matchToBars
	with CAPlayableLength as parameter, which is used at midi keyboard input. This functions probably could be
	merged.
*/
QList<CAPlayableLength> CAPlayableLength::matchToBars( int timeLength, int timeStart, CABarline *lastBarline, CATimeSignature *ts, int dotsLimit ) {

	QList<CAPlayableLength> list;
	// default time signature is 4/4
	int barLength = CAPlayableLength::playableLengthToTimeLength( CAPlayableLength::Quarter ) * 4;
	if (ts) {
		int beat = ts->beat();
		switch (beat) {
		case 4:
		case 2:
		case 8:
		case 16:
		case 1:
		case 32:	break;
		default:	return list; // If something is strange or undoable we prepare for returning an empty list!
		}
		barLength = CAPlayableLength::playableLengthToTimeLength(
				CAPlayableLength( static_cast<CAPlayableLength::CAMusicLength>(ts->beat()) ) ) * ts->beats();
	}
	int barRest = ( lastBarline ? lastBarline->timeStart() : 0 ) + barLength - timeStart;

	// no change when bar lengths are bogus
	if (lastBarline && ts && (lastBarline->timeStart() < ts->timeStart()))
		barRest = 0;
	if (ts && (timeStart == ts->timeStart()))
		barRest = 0;
	if (barRest < 0 || barRest > barLength)
		barRest = 0;

	int noteLen = timeLength;

	// now we really do a split
	int tSplit = barRest ? barRest : barLength;
	bool longNotesFirst = barRest ? false : true;
	while (noteLen) {
		tSplit = tSplit > noteLen ? noteLen : tSplit;
		list << timeLengthToPlayableLengthList( tSplit, longNotesFirst, dotsLimit );
		noteLen -= tSplit;
		tSplit = noteLen > barLength ? barLength : noteLen;
		longNotesFirst = true;
	}

	return list;
}

