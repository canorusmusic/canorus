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
		enum CAStemDirection {
			StemNeutral,	// Up if under the middle line, down if above the middle line
			StemUp,
			StemDown,
			StemPreferred		// Use the voice's prefered direction
		};
		
		CANote(CAPlayableLength length, CAVoice *voice, int pitch, signed char accs, int timeStart, int dotted=0);
		CANote *clone();
		
		CAPlayableLength noteLength() { return _playableLength; }
		int pitch() { return _pitch; }
		int accidentals() { return _accs; }
		CAStemDirection stemDirection() { return _stemDirection; }
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
			inline CASlur *tieStart() { return _tieStart; }
	CAStemDirection         actualStemDirection();
	CASlur::CASlurDirection actualSlurDirection();
			
	inline CASlur *tieEnd() { return _tieEnd; }
	inline CASlur *slurStart() { return _slurStart; }
	inline CASlur *slurEnd() { return _slurEnd; }
	inline CASlur *phrasingSlurStart() { return _phrasingSlurStart; }
	inline CASlur *phrasingSlurEnd() { return _phrasingSlurEnd; }
	
	inline void setTieStart( CASlur *tieStart ) { _tieStart = tieStart; }
	inline void setTieEnd( CASlur *tieEnd ) { _tieEnd = tieEnd; }
	inline void setSlurStart( CASlur *slurStart ) { _slurStart = slurStart; }
	inline void setSlurEnd( CASlur *slurEnd ) { _slurEnd = slurEnd; }
	inline void setPhrasingSlurStart( CASlur *pSlurStart ) { _phrasingSlurStart = pSlurStart; }
	inline void setPhrasingSlurEnd( CASlur *pSlurEnd ) { _phrasingSlurEnd = pSlurEnd; }
		
		static const QString generateNoteName(int pitch, int accs);
		
		int compare(CAMusElement* elt);
		static int pitchToMidiPitch(int pitch, int acc);
		static int midiPitchToPitch(int midiPitch);
		inline unsigned char midiPitch() { return _midiPitch; }
		void setMidiPitch(unsigned char pitch) { _midiPitch = pitch; }
};
