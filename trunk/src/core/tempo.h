/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef TEMPO_H_
#define TEMPO_H_

#include "core/mark.h"
#include "core/playable.h"

class CATempo : public CAMark {
public:
	CATempo( CAPlayable::CAPlayableLength l, int dotted, int bpm, CAMusElement *m );
	virtual ~CATempo();
	
	inline int bpm() { return _bpm; }
	inline void setBpm( int bpm ) { _bpm = bpm; }
	inline CAPlayable::CAPlayableLength playableLength() { return _playableLength; }
	inline void setPlayableLength( CAPlayable::CAPlayableLength l ) { _playableLength = l; }
	inline int dotted() { return _dotted; }
	inline void setDotted( int dotted ) { _dotted = dotted; }
	
private:
	CAPlayable::CAPlayableLength _playableLength;
	int _dotted;
	int _bpm; // beats per minute
};

#endif /* TEMPO_H_ */
