/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef PLAYABLE_H_
#define PLAYABLE_H_

#include "core/muselement.h"
#include "core/staff.h"
#include "core/playablelength.h"

class CAVoice;

class CAPlayable : public CAMusElement {
public:
	CAPlayable( CAPlayableLength length, CAVoice *voice, int timeStart );
	virtual ~CAPlayable();
	virtual CAPlayable *clone( CAVoice *voice ) = 0;
	
	inline CAPlayableLength& playableLength() { return _playableLength; }
	inline void setPlayableLength( CAPlayableLength& l ) { _playableLength = l; calculateTimeLength(); }
	
	inline CAVoice *voice() { return _voice; }
	void setVoice(CAVoice *v);
	inline CAStaff* staff() { return static_cast<CAStaff*>(_context); }
	
protected:
	void calculateTimeLength();
	
	CAPlayableLength _playableLength;
	CAVoice *_voice;
};
#endif /* PLAYABLE_H_ */
