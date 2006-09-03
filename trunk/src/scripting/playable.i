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
		CAPlayable(CAVoice *voice, int timeStart, int timeLength);
		inline unsigned char midiPitch();
		void setMidiPitch(unsigned char pitch);
		inline int midiLength();
		void setMidiLength(int length);
		CAVoice *voice();
		void setVoice(CAVoice *v);

		static int pitchToMidiPitch(int pitch, int acc);
		static int midiPitchToPitch(int midiPitch);
};
