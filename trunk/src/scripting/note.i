/** @file scripting/note.i
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

%{
#include "core/note.h"
%}

/**
 * Swig implementation of CANote.
 */
%rename(Note) CANote;
class CANote : public CAPlayable {
	public:
		enum CANoteLength {
			None = -1,
			Breve = 0,
			Whole = 1,
			Half = 2,
			Quarter = 4,
			Eighth = 8,
			Sixteenth = 16,
			ThirtySecond = 32,
			SixtyFourth = 64,
			HundredTwentyEighth = 128
		};
		
		CANote(CANoteLength length, CAVoice *voice, int pitch, signed char accs, int timeStart, int timeLength=0);
		CANote *clone();
		
		CANoteLength noteLength();
		int pitch();
		const QString pitchML();	///Compose the note pitch name for the CanorusML format
		const QString lengthML();	///Compose the note length for the CanorusML format
		void setPitch(int pitch);
		int notePosition();

		static const QString generateNoteName(int pitch);
};
