/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/playable.h"
#include "core/voice.h"
#include "core/staff.h"

/*!
	\class CAPlayable
	\brief Playable instances of music elements.
	
	CAPlayable class represents a base class for all the music elements which are
	playable (_timeLength property is greater than 0). It brings other properties
	like the music length (whole, half, quarter etc.), number of dots and instead
	of contexts, playable elements voices for their parent objects.
	
	Notes and rests inherit this class.
	
	\sa CAMusElement, CAPlayableType
*/

/*!
	Creates a new playable element with playable length \a length, \a voice, \a timeStart
	and number of dots \a dotted.
	
	\sa CAPlayableLength, CAVoice, CAMusElement
*/
CAPlayable::CAPlayable(CAPlayableLength length, CAVoice *voice, int timeStart, int dotted)
 : CAMusElement(voice?voice->staff():0, timeStart, 0) {
	setVoice( voice );
	setPlayableLength( length );	
	_dotted = dotted;
	setTimeLength( playableLengthToTimeLength(length, dotted) );
}

/*!
	Destroys the playable element.
*/
CAPlayable::~CAPlayable() {
	if (voice())
		voice()->remove( this );
}

void CAPlayable::setVoice(CAVoice *voice) {
	_voice = voice; _context = voice?voice->staff():0;
}

/*!
	Sets the playable element having \a dotted dots and returns the difference of the previous
	and new time lengths in absolute time units.
	
	This function also sets the new timeLength.
	
	\sa dotted()
*/
int CAPlayable::setDotted(int dotted) {
	// calculate the original note length
	float factor = 1.0, delta=0.5;
	for (int i=0; i<this->dotted(); i++, factor+=delta, delta/=2);
	int origLength = (int)( timeLength() / factor );
	
	// calculate and set the new note length
	_dotted = dotted;	
	factor = 1.0, delta=0.5;
	for (int i=0; i<dotted; i++, factor+=delta, delta/=2);                // calculate the length factor out of number of dots

	return (_timeLength - (_timeLength = qRound(origLength * factor)))*-1; // return delta of the new and old timeLengths, set the new timeLength
}

CAPlayable::CAPlayableLength CAPlayable::playableLengthFromString(const QString length) {
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

const QString CAPlayable::playableLengthToString(CAPlayableLength length) {
	switch (length) {
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
	Converts internal enum playableLength to logical timeLength.
*/
const int CAPlayable::playableLengthToTimeLength( CAPlayableLength length, int dotted ) {
	int timeLength;
	
	switch (length) {
		case CAPlayable::HundredTwentyEighth:
			timeLength = 8;
			break;
		case CAPlayable::SixtyFourth:
			timeLength = 16;
			break;
		case CAPlayable::ThirtySecond:
			timeLength = 32;
			break;
		case CAPlayable::Sixteenth:
			timeLength = 64;
			break;
		case CAPlayable::Eighth:
			timeLength = 128;
			break;
		case CAPlayable::Quarter:
			timeLength = 256;
			break;
		case CAPlayable::Half:
			timeLength = 512;
			break;
		case CAPlayable::Whole:
			timeLength = 1024;
			break;
		case CAPlayable::Breve:
			timeLength = 2048;
			break;
		default:            // This should never occur!
			timeLength = 0;
			break;
	}
	
	float factor = 1.0, delta=0.5;
	for (int i=0; i<dotted; i++, factor+=delta, delta/=2);  // calculate the length factor out of number of dots
	timeLength = qRound(timeLength*factor);                 // increase the time length for the factor
	
	return timeLength;
}
