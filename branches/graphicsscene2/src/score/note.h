/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef NOTE_H_
#define NOTE_H_

#include "score/muselement.h"
#include "score/playable.h"
#include "score/slur.h"
#include "score/diatonicpitch.h"

class CAVoice;

class CANote : public CAPlayable {
public:
	enum CAStemDirection {
		StemUndefined = -1,
		StemNeutral,
		StemUp,
		StemDown,
		StemPreferred // voice's direction
	};

	CANote( CADiatonicPitch pitch, CAPlayableLength length, CAVoice *voice, int timeStart, int timeLength=-1 );
	CANote *clone(CAVoice* voice=0);

	virtual ~CANote();

	CAPlayableLength noteLength() { return _playableLength; }

	inline CADiatonicPitch& diatonicPitch() { return _diatonicPitch; }
	inline void setDiatonicPitch( CADiatonicPitch pitch ) {
		_diatonicPitch = pitch;
		updateTies();
	}
	inline int midiPitch() { return _diatonicPitch.midiPitch(); }

	CAStemDirection stemDirection() { return _stemDirection; }
	void setStemDirection(CAStemDirection direction);

	int notePosition();

	inline CASlur *tieStart() { return _tieStart; }
	inline CASlur *tieEnd() { return _tieEnd; }
	inline CASlur *slurStart() { return _slurStart; }
	inline CASlur *slurEnd() { return _slurEnd; }
	inline CASlur *phrasingSlurStart() { return _phrasingSlurStart; }
	inline CASlur *phrasingSlurEnd() { return _phrasingSlurEnd; }

	CAStemDirection         actualStemDirection();
	CASlur::CASlurDirection actualSlurDirection();

	inline void setTieStart( CASlur *tieStart ) { _tieStart = tieStart; }
	inline void setTieEnd( CASlur *tieEnd ) { _tieEnd = tieEnd; }
	inline void setSlurStart( CASlur *slurStart ) { _slurStart = slurStart; }
	inline void setSlurEnd( CASlur *slurEnd ) { _slurEnd = slurEnd; }
	inline void setPhrasingSlurStart( CASlur *pSlurStart ) { _phrasingSlurStart = pSlurStart; }
	inline void setPhrasingSlurEnd( CASlur *pSlurEnd ) { _phrasingSlurEnd = pSlurEnd; }

	void updateTies();

	bool isPartOfChord();
	bool isLastInChord();
	bool isFirstInChord();
	QList<CANote*> getChord();

	bool forceAccidentals() { return _forceAccidentals; }
	void setForceAccidentals(bool force) { _forceAccidentals = force; }

	static const QString generateNoteName(int pitch, int accs);

	static const QString stemDirectionToString(CAStemDirection);
	static CAStemDirection stemDirectionFromString(const QString);

	int compare(CAMusElement* elt);

private:
	CADiatonicPitch _diatonicPitch;
	CAStemDirection _stemDirection;
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
