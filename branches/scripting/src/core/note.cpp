/** @file note.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "core/note.h"
#include "core/voice.h"
#include "core/staff.h"
#include "core/clef.h"
#include <iostream>
CANote::CANote(CANoteLength length, CAVoice *voice, int pitch, signed char accs, int timeStart, int timeLength)
 : CAPlayable(voice, timeStart, timeLength) {
	_musElementType = CAMusElement::Note;
	_noteLength = length;
	_accs = accs;

	_pitch = pitch;
	_midiPitch = CAPlayable::pitchToMidiPitch(pitch, _accs);

	if (timeLength)
		_timeLength = timeLength;
	else {
		switch (length) {
			case CANote::HundredTwentyEighth:
				_timeLength = 8;
				break;
			case CANote::SixtyFourth:
				_timeLength = 16;
				break;
			case CANote::ThirtySecond:
				_timeLength = 32;
				break;
			case CANote::Sixteenth:
				_timeLength = 64;
				break;
			case CANote::Eighth:
				_timeLength = 128;
				break;
			case CANote::Quarter:
				_timeLength = 256;
				break;
			case CANote::Half:
				_timeLength = 512;
				break;
			case CANote::Whole:
				_timeLength = 1024;
				break;
			case CANote::Breve:
				_timeLength = 2048;
				break;
		}
	}
	
	_midiLength = _timeLength;

	calculateNotePosition();
}

CANote *CANote::clone() {
	CANote *d = new CANote(_noteLength, _voice, _pitch, _accs, _timeStart);
	
	return d;
}

void CANote::calculateNotePosition() {
	CAClef *clef = (_voice?_voice->getClef(this):0);
	
	_notePosition = _pitch + (clef?clef->c1():-2) - 28;
}

const QString CANote::generateNoteName(int pitch) {
	QString name;
	
	name = (char)((pitch+2)%7 + 'a');
	if (pitch < 21)
		name = name.toUpper();
	
	for (int i=0; i<(pitch-21)/7; i++)
		name.append('\'');
	
	if (pitch<14)
		name.append(',');
	if (pitch<7)
		name.append(',');
	
	return name;
}

const QString CANote::pitchML() {
	QString name;
	
	name = (char)((_pitch+2)%7 + 'a');

	for (int i=0; i < _accs; i++)
		name += "is";	//append as many -is-es as necessary
	
	for (int i=0; i > _accs; i--) {
		if ( ((name == "e") || (name == "a")) && (i==0) )
			name += "s";	//for e and a, only append single -s the first time
		else
			name += "es";	//otherwise, append normally as many es-es as necessary
	}
	
	return name;
}

const QString CANote::lengthML() {
	QString length;
	switch (_noteLength) {
		case CANote::Breve:
			length = "0";
			break;
		case CANote::Whole:
			length = "1";
			break;
		case CANote::Half:
			length = "2";
			break;
		case CANote::Quarter:
			length = "4";
			break;
		case CANote::Eighth:
			length = "8";
			break;
		case CANote::Sixteenth:
			length = "16";
			break;
		case CANote::ThirtySecond:
			length = "32";
			break;
		case CANote::SixtyFourth:
			length = "64";
			break;
		case CANote::HundredTwentyEighth:
			length = "128";
			break;
	}
	
	return length;
}
