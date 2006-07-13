/** @file note.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef NOTE_H_
#define NOTE_H_

#include "muselement.h"
#include "playable.h"

class CAVoice;

class CANote :  public CAMusElement, public CAPlayable {
	public:
		enum CANoteLength {
			Brevis,
			Whole,
			Half,
			Quarter,
			Eighth,
			Sixteenth,
			ThirtySecondth,
			SixtyFourth,
			HundredTwentyEighth
		};
		
		CANote(CANoteLength length, CAVoice *voice, int timeStart, int timeLength=0);
		CANote *clone();
		
		CANoteLength noteLength() { return _noteLength; }
		CAVoice *voice() { return _voice; }

	private:
		CANoteLength _noteLength;
		CAVoice *_voice;
};
#endif /*NOTE_H_*/

