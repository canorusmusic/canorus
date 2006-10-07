/** @file note.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef NOTE_H_
#define NOTE_H_

#include "core/muselement.h"
#include "core/playable.h"

class CAVoice;

class CANote : public CAPlayable {
	public:
		CANote(CAPlayableLength length, CAVoice *voice, int pitch, signed char accs, int timeStart, int timeLength=0);
		CANote *clone();
		
		CAPlayableLength noteLength() { return _playableLength; }
		int pitch() { return _pitch; }
		int accs() { return _accs; }
		const QString pitchML();	///Compose the note pitch name for the CanorusML format
		const QString lengthML();	///Compose the note length for the CanorusML format
		void setPitch(int pitch);
		void setAccs(int accs);
		int notePosition() { return _notePosition; }
		
		/**
		 * Return True, if the note is part of a chord, False otherwise.
		 */
		bool isPartOfTheChord();
		
		/**
		 * Return True, if the note is the topest in the chord, False otherwise.
		 */
		bool isLastInTheChord();
		
		/**
		 * Return True, if the note is the lowest in the chord, False otherwise.
		 */
		bool isFirstInTheChord();
		
		/**
		 * Return a list of notes with the same start time - the whole chord.
		 * Return a single element in the list - only the note itself, if the note isn't part of the chord.
		 * 
		 * @return All notes in the chord which this note is part of.
		 */
		QList<CANote*> chord();
		
		static const QString generateNoteName(int pitch);
		
		int compare(CAMusElement* elt);

	private:
		/**
		 * Depending on the clef, calculate note position in the staff.
		 */
		void calculateNotePosition();
	
		int _pitch;	///note pitch in logical units. 0 = C,, , 1 = Sub-Contra D,, , 56 = c''''' etc.
		int _accs;	///note accidentals. 0 = neutral, 1 = sharp, -1 = flat etc.
		int _notePosition;	///note location in the staff. 0 first line, 1 first space, -2 first ledger line below the staff
};
#endif /*NOTE_H_*/

