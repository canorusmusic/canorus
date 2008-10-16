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
	To make this computation fast we take in account that note durations are
	a binary representation of the time duration. Restrictions are the number
	of dots allowed for small and big durations.

	Limitations: Maximum four dots per note, bar the smallest resulting time duration
	is SixtyFourth;
*/
const CAPlayableLength CAPlayableLength::timeLengthToPlayableLength( int t ) {

	// We compute the biggest note not longer than t:
	int x,d;
	int dotsAllowed = -1;	// 128th and 64th no dots;
	int musLen = HundredTwentyEighth;
	for ( x = playableLengthToTimeLength( CAMusicLength(musLen) ); x < playableLengthToTimeLength( Breve ); x *= 2 ) {
		if ( x*2 > t ) break;
		dotsAllowed++;
		musLen /= 2;
	}
	// maybe we found the match without dots
	//std::cout<<"pre        t "<<t<<" "<<x<<" "<<t<<" "<<dotsAllowed<<std::endl;
	if ( x == t )
		return CAPlayableLength( CAMusicLength(musLen) );

	//std::cout<<"dot"<<std::endl;
	// We add dots
	int dval = x;
	for (d = 0; d <= dotsAllowed && d <= 4 ; d++) {
		//std::cout<<"          in         "<<d<<" "<<t<<" "<<(x+(x>>d))<<std::endl;
		if (x+dval/2 > t) break;
		dval /=2;
		x += dval;
	}
	return CAPlayableLength( CAMusicLength(musLen), d );
}

/*!
	To make this computation fast we take in account that note durations are
	a binary representation of the time duration. Restrictions are the number
	of dots allowed for small and big durations.

	Limitations: Maximum four dots per note, bar the smallest resulting time duration
	is SixtyFourth;
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
				pl << CAPlayableLength( CAMusicLength( (1<<logCurrentMusLenPlusOne)/2 ));
				dots = maxDots;
				findNote = maxDots > 0 ? false : true;
			} else {
				findNote = true;
			}
		} else {
			// try to find a dot here
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
	//std::cout<<" Analyse: "<<( lastBarline ? lastBarline->timeStart() : 0 )<<" "<<barLength<<" "<<timeStart<<std::endl;
	// no change when bar lengths bogus
	if (barRest < 0 || barRest > barLength) return unchanged;

	int noteLen = len.playableLengthToTimeLength( len );
	int orig = noteLen;

	// split is ovbious, either the rest of the old bar or completely new bars
	QList<CAPlayableLength> list;
	int tSplit = barRest ? barRest : barLength;
	//std::cout<<"  vorm split: "<<tSplit<<" "<<noteLen<<std::endl;
	do {
		if (noteLen) {
			tSplit = tSplit > noteLen ? noteLen : tSplit;
			list << timeLengthToPlayableLength( tSplit );
			//std::cout<<" -- -- "<<playableLengthToTimeLength(list.back());
			int lt = playableLengthToTimeLength( list.back() );
			noteLen -= lt;
			tSplit -= lt;
			if (!tSplit)
				tSplit = noteLen > barLength ? barLength : noteLen;
		} else {
			// list << timeLengthToPlayableLength( noteLen );
			//std::cout<<" -- -- "<<playableLengthToTimeLength(list.back());
			break;
		}
	} while (true);
	//std::cout<<" war: "<<orig<<" rest: "<<barRest<<" Taktl: "<<barLength<<" zus.: "<<list.size()<<std::endl;
	
	return list;
}

