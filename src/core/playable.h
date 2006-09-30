/** @file playable.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef PLAYABLE_H_
#define PLAYABLE_H_

#include "core/muselement.h"
#include "core/staff.h"

class CAVoice;

class CAPlayable : public CAMusElement {
	public:
		enum CAPlayableLength {
			None = -1,
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
		
		CAPlayable(CAVoice *voice, int timeStart, int timeLength);
		inline unsigned char midiPitch() { return _midiPitch; }
		void setMidiPitch(unsigned char pitch) { _midiPitch = pitch; }
		
		inline int midiLength() { return _midiLength; }
		void setMidiLength(int length) { _midiLength = length; }
		
		inline CAPlayableLength playableLength() { return _playableLength; }
		
		CAVoice *voice() { return _voice; }
		void setVoice(CAVoice *v);

		static int pitchToMidiPitch(int pitch, int acc);
		static int midiPitchToPitch(int midiPitch);
	
	protected:
		int _midiLength;
		unsigned char _midiPitch;
		CAPlayableLength _playableLength;
		CAVoice *_voice;
};

#endif /*PLAYABLE_H_*/
