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
		CANote(CAPlayableLength length, CAVoice *voice, int pitch, signed char accs, int timeStart, int dotted=0);
		CANote *clone();
		
		CAPlayableLength noteLength();
		int pitch();
		int accidentals();
		const QString pitchML();	///Compose the note pitch name for the CanorusML format
		const QString lengthML();	///Compose the note length for the CanorusML format
		void setPitch(int pitch);
		void setAccidentals(int accs);
		int notePosition();
		
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
		
		bool forceAccidentals();
		void setForceAccidentals(bool force);
		
		static const QString generateNoteName(int pitch, int accs);
		
		int compare(CAMusElement* elt);

};
