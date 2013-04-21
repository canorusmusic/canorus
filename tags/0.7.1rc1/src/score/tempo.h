/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef TEMPO_H_
#define TEMPO_H_

#include "score/mark.h"
#include "score/playable.h"

class CATempo : public CAMark {
public:
	CATempo( CAPlayableLength l, int bpm, CAMusElement *m );
	virtual ~CATempo();

	CATempo *clone(CAMusElement* elt=0);
	int compare( CAMusElement *elt );

	inline int bpm() { return _bpm; }
	inline void setBpm( int bpm ) { _bpm = bpm; }
	inline CAPlayableLength beat() { return _beat; }
	inline void setBeat( CAPlayableLength l ) { _beat = l; }
	inline int beatDotted() { return _beatDotted; }
	inline void setBeatDotted( int dotted ) { _beatDotted = dotted; }

private:
	CAPlayableLength _beat;
	int _beatDotted;
	int _bpm; // beats per minute
};

#endif /* TEMPO_H_ */
