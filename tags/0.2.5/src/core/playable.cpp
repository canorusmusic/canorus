/* 
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
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
	_voice = voice;
	_playableLength = length;
	
	switch (length) {
		case CAPlayable::HundredTwentyEighth:
			_timeLength = 8;
			break;
		case CAPlayable::SixtyFourth:
			_timeLength = 16;
			break;
		case CAPlayable::ThirtySecond:
			_timeLength = 32;
			break;
		case CAPlayable::Sixteenth:
			_timeLength = 64;
			break;
		case CAPlayable::Eighth:
			_timeLength = 128;
			break;
		case CAPlayable::Quarter:
			_timeLength = 256;
			break;
		case CAPlayable::Half:
			_timeLength = 512;
			break;
		case CAPlayable::Whole:
			_timeLength = 1024;
			break;
		case CAPlayable::Breve:
			_timeLength = 2048;
			break;
		default:				// This should never happen!
			_timeLength = 0;
			break;
	}
	
	float factor = 1.0, delta=0.5;
	for (int i=0; i<dotted; i++, factor+=delta, delta/=2);	// calculate the length factor out of number of dots
	_timeLength = (int)(_timeLength*factor+0.5);	// increase the time length for the factor
	
	_midiLength = _timeLength;
	_dotted = dotted;
}

/*!
	Destroys the playable element.
*/
CAPlayable::~CAPlayable() {
}

void CAPlayable::setVoice(CAVoice *voice) {
	_voice = voice; _context = voice->staff();
}

/*!
	Sets the playable element having \a dotted dots and returns the difference of the previous
	and new time lengths in absolute time units.
	
	\sa dotted()
*/
int CAPlayable::setDotted(int dotted) {
	// calculate the original note length
	float factor = 1.0, delta=0.5;
	for (int i=0; i<_dotted; i++, factor+=delta, delta/=2);
	int origLength = (int)(_timeLength / factor);
	
	// calculate and set the new note length
	_dotted = dotted;	
	factor = 1.0, delta=0.5;
	for (int i=0; i<_dotted; i++, factor+=delta, delta/=2);	//calculate the length factor out of number of dots

	return (_timeLength - (_timeLength = (int)(origLength * factor)))*-1;	//return delta of the new and old timeLengths, set the new timeLength
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
