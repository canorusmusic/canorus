/** @file scripting/playable.i
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

%{
#include "core/playable.h"
%}

/**
 * Swig implementation of CAPlayable.
 */
%rename(Playable) CAPlayable;
class CAPlayable : public CAMusElement {
	public:
		enum CAPlayableLength {
			Undefined = -1,
			Breve = 0,
			Whole = 1,
			Half = 2,
			Quarter = 4,
			Eighth = 8,
			Sixteenth = 16,
			ThirtySecond = 32,
			SixtyFourth = 64,
			HundredTwentyEighth = 128
		};
		
		CAPlayable(CAPlayableLength length, CAVoice *voice, int timeStart, int dotted=0);
		~CAPlayable();
		
		inline CAPlayableLength playableLength() { return _playableLength; }
		
		int dotted();
		int setDotted(int dotted);
		
		CAVoice *voice() { return _voice; }
		void setVoice(CAVoice *v);
		CAStaff *staff();
};