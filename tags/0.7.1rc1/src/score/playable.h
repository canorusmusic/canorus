/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef PLAYABLE_H_
#define PLAYABLE_H_

#include "score/muselement.h"
#include "score/staff.h"
#include "score/playablelength.h"
#include "score/tuplet.h"

class CAVoice;

class CAPlayable : public CAMusElement {
public:
	CAPlayable( CAPlayableLength length, CAVoice *voice, int timeStart, int timeLength=-1 );
	virtual ~CAPlayable();

	inline CAPlayableLength& playableLength() { return _playableLength; }
	inline void setPlayableLength( CAPlayableLength& l ) { _playableLength = l; }
	virtual CAPlayable* clone(CAContext* context) { CAPlayable* pl = clone(); pl->setContext(context); return pl; }
	virtual CAPlayable* clone(CAVoice* voice=0)=0;

	CATuplet *tuplet() { return _tuplet; }
	void setTuplet( CATuplet *t ) { _tuplet = t; }

	inline CAVoice *voice() { return _voice; }
	void setVoice(CAVoice *v);
	inline CAStaff* staff() { return static_cast<CAStaff*>(_context); }

	inline bool isFirstInTuplet() { return ( _tuplet && _tuplet->firstNote()==this); }
	inline bool isLastInTuplet() { return ( _tuplet && _tuplet->lastNote()==this); }

	void resetTime();
	void calculateTimeLength();

protected:
	CAPlayableLength _playableLength;
	CAVoice *_voice;
	CATuplet *_tuplet;
};
#endif /* PLAYABLE_H_ */
