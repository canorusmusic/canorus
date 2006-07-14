/** @file note.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "note.h"
#include "voice.h"
#include "staff.h"
#include "clef.h"
#include <iostream>
CANote::CANote(CANoteLength length, CAVoice *voice, int pitch, int timeStart, int timeLength)
 : CAMusElement(voice->staff(), timeStart, timeLength) {
	_musElementType = CAMusElement::Note;
	_noteLength = length;
	_pitch = pitch;

	if (timeLength)
		_timeLength = timeLength;
	else
		_timeLength = 256;	//TODO: Set the note length for every noteLengthType possibility (NoteLengthType can have arbitrary exact value)

	_voice = voice;

	calculateNotePosition();
}

CANote *CANote::clone() {
	CANote *d = new CANote(_noteLength, _voice, _pitch, _timeStart);
	
	return d;
}

void CANote::calculateNotePosition() {
	CAClef *clef = _voice->getClef(this);
	
	_notePosition = _pitch + (clef?clef->c1():-2) - 28;
}
