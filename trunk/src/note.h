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
		
		CANote(CANoteLength length);
		CANote *clone();
		
		CANoteLength noteLength() { return _noteLength; }

	private:
		CANoteLength _noteLength;
};
#endif /*NOTE_H_*/

