/*
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 *
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#ifndef NOTE_H_
#define NOTE_H_

#include "core/muselement.h"
#include "core/playable.h"
#include "core/slur.h"

class CAVoice;

class CANote : public CAPlayable {
public:
	enum CAStemDirection {
		StemNeutral,
		StemUp,
		StemDown,
		StemPrefered // voice's direction
	};
	
	CANote(CAPlayableLength length, CAVoice *voice, int pitch, signed char accs, int timeStart, int dotted=0);
	CANote *clone();
	
	virtual ~CANote();
	
	CAPlayableLength noteLength() { return _playableLength; }
	
	int pitch() { return _pitch; }
	void setPitch(int pitch);
	
	int accidentals() { return _accs; }
	void setAccidentals(int accs) { _accs = accs; };
	
	CAStemDirection stemDirection() { return _stemDirection; }
	void setStemDirection(CAStemDirection direction);
	
	int notePosition() { return _notePosition; }
	
	inline unsigned char midiPitch() { return _midiPitch; }
	void setMidiPitch(unsigned char pitch) { _midiPitch = pitch; }
	
	inline CASlur *tieStart() { return _tieStart; }
	inline CASlur *tieEnd() { return _tieEnd; }
	inline CASlur *slurStart() { return _slurStart; }
	inline CASlur *slurEnd() { return _slurEnd; }
	inline CASlur *phrasingSlurStart() { return _phrasingSlurStart; }
	inline CASlur *phrasingSlurEnd() { return _phrasingSlurEnd; }
	
	CAStemDirection         determineStemDirection();
	CASlur::CASlurDirection determineSlurDirection();
	
	inline void setTieStart( CASlur *tieStart ) { _tieStart = tieStart; }
	inline void setTieEnd( CASlur *tieEnd ) { _tieEnd = tieEnd; }
	inline void setSlurStart( CASlur *slurStart ) { _slurStart = slurStart; }
	inline void setSlurEnd( CASlur *slurEnd ) { _slurEnd = slurEnd; }
	inline void setPhrasingSlurStart( CASlur *pSlurStart ) { _phrasingSlurStart = pSlurStart; }
	inline void setPhrasingSlurEnd( CASlur *pSlurEnd ) { _phrasingSlurEnd = pSlurEnd; }
	
	void updateSlurDirections();
	void updateTies();
	
	bool isPartOfTheChord();
	bool isLastInTheChord();
	bool isFirstInTheChord();
	QList<CANote*> chord();
	
	bool forceAccidentals() { return _forceAccidentals; }
	void setForceAccidentals(bool force) { _forceAccidentals = force; }
	
	static const QString generateNoteName(int pitch, int accs);
	
	static const QString stemDirectionToString(CAStemDirection);
	static CAStemDirection stemDirectionFromString(const QString);
	
	static int pitchToMidiPitch(int pitch, int acc);
	static int midiPitchToPitch(int midiPitch);
	
	int compare(CAMusElement* elt);
	
private:
	void calculateNotePosition();
	unsigned char _midiPitch;
	
	int _pitch;
	int _accs;
	CAStemDirection _stemDirection;
	int _notePosition; // Note location in the staff. 0 first line, 1 first space, -2 first ledger line below the staff etc.
	bool _forceAccidentals; // Always draw notes accidentals.
	
	////////////////////
	// Slurs and ties //
	////////////////////
	CASlur *_tieStart;
	CASlur *_tieEnd;
	CASlur *_slurStart;
	CASlur *_slurEnd;
	CASlur *_phrasingSlurStart;
	CASlur *_phrasingSlurEnd;
};
#endif /* NOTE_H_*/
