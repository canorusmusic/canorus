/** @file playable.cp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "core/playable.h"
#include "core/voice.h"
#include "core/staff.h"

CAPlayable::CAPlayable(CAVoice *voice, int timeStart, int timeLength)
 : CAMusElement(voice?voice->staff():0, timeStart, timeLength) {
	_voice = voice;
	_playable = true;
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
