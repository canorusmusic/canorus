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

CANote::CANote(CANoteLength length, CAVoice *voice, int timeStart, int timeLength) : CAMusElement(voice->staff(), timeLength) {
	_noteLength = length;

	if (timeLength)
		_timeLength = timeLength;
	else
		_timeLength = length;	//TODO: Set the note length for every noteLengthType possibility (NoteLengthType can have arbitrary exact value)

	_voice = voice;
}

CANote *CANote::clone() {
	CANote *d = new CANote(_noteLength, _voice, _timeStart);
	
	return d;
}
