/** @file interface/playable.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "core/playable.h"
#include "core/voice.h"
#include "core/staff.h"

CAPlayable::CAPlayable(CAPlayableLength length, CAVoice *voice, int timeStart, int dotted)
 : CAMusElement(voice?voice->staff():0, timeStart, 0) {
	_voice = voice;
	_playable = true;
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
		default:				//This should never happen!
			_timeLength = 0;
			break;
	}
	
	float factor = 1.0, delta=0.5;
	for (int i=0; i<dotted; i++, factor+=delta, delta/=2);	//calculate the length factor out of number of dots
	_timeLength = (int)(_timeLength*factor+0.5);	//increase the time length for the factor
	
	_midiLength = _timeLength;
	_dotted = dotted;
}

int CAPlayable::pitchToMidiPitch(int pitch, int acc) {
	float step = (float)12/7;
	
	//+0.3 - rounding factor for 7/12 that exactly underlays every tone in octave
	//+0.5 - casting to int cut-off the decimal part, not round it. In order to round it - add 0.5
	//+12 - our logical pitch starts at Sub-contra C, midi counting starts one octave lower
	return (int)(pitch * step + 0.3 + 0.5 + 12) + acc;
}

int CAPlayable::midiPitchToPitch(int midiPitch) {
	return 0; //TODO
}

void CAPlayable::setVoice(CAVoice *voice) {
	_voice = voice; _context = voice->staff();
}

int CAPlayable::setDotted(int dotted) {
	//calculate the original note length
	float factor = 1.0, delta=0.5;
	for (int i=0; i<_dotted; i++, factor+=delta, delta/=2);
	int origLength = (int)(_timeLength / factor + 0.5);
	
	//calculate and set the new note length
	factor = 1.0, delta=0.5;
	for (int i=0; i<_dotted; i++, factor+=delta, delta/=2);	//calculate the length factor out of number of dots
	_dotted = dotted;	

	return (_timeLength - (_timeLength = (int)(origLength * factor + 0.5)))*-1;	//return delta of the new and old timeLengths, set the new timeLength
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
