/** @file core/note.h
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
		enum CAStemDirection {
			StemNeutral,	// Up if under the middle line, down if above the middle line
			StemUp,
			StemDown,
			StemPrefered	// Use the voice's prefered direction
		};
		
		CANote(CAPlayableLength length, CAVoice *voice, int pitch, signed char accs, int timeStart, int dotted=0);
		CANote *clone();
		
		CAPlayableLength noteLength() { return _playableLength; }
		int pitch() { return _pitch; }
		int accidentals() { return _accs; }
		CAStemDirection stemDirection() { return _stemDirection; }
		/** OBSOLETE */
		const QString pitchML();	/// Compose the note pitch name for the CanorusML format
		/** OBSOLETE */
		const QString lengthML();	/// Compose the note length for the CanorusML format
		void setPitch(int pitch);
		void setAccidentals(int accs) { _accs = accs; };
		void setStemDirection(CAStemDirection direction) { _stemDirection = direction; }
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
		
		bool forceAccidentals() { return _forceAccidentals; }
		void setForceAccidentals(bool force) { _forceAccidentals = force; }
		
		static const QString generateNoteName(int pitch, int accs);		// Used for showing the note pitch status bar
		static const QString stemDirectionToString(CAStemDirection);	// From StemDirection to String
		static CAStemDirection stemDirectionFromString(const QString);	// From String to StemDirection
		
		int compare(CAMusElement* elt);

	private:
		/**
		 * Depending on the clef, calculate note position in the staff.
		 */
		void calculateNotePosition();
	
		int _pitch;	/// Note pitch in logical units. 0 = C,, , 1 = Sub-Contra D,, , 56 = c''''' etc.
		int _accs;	/// Note accidentals. 0 = neutral, 1 = sharp, -1 = flat etc.
		CAStemDirection _stemDirection;	/// Direction of the note's stem, if any. See CAStemDirection.
		int _notePosition;	/// Note location in the staff. 0 first line, 1 first space, -2 first ledger line below the staff
		bool _forceAccidentals;	/// Always draw notes accidentals.
};
#endif /*NOTE_H_*/

