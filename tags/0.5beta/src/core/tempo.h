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
	
	CAMusElement *clone();
	int compare( CAMusElement *elt );
	
	inline int bpm() { return _bpm; }
	inline void setBpm( int bpm ) { _bpm = bpm; }
	inline CAPlayable::CAPlayableLength beat() { return _beat; }
	inline void setBeat( CAPlayable::CAPlayableLength l ) { _beat = l; }
	inline int beatDotted() { return _beatDotted; }
	inline void setBeatDotted( int dotted ) { _beatDotted = dotted; }
	
private:
	CAPlayable::CAPlayableLength _beat;
	int _beatDotted;
	int _bpm; // beats per minute
};

#endif /* TEMPO_H_ */
