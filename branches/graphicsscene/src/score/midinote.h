/*!
	Copyright (c) 2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef MIDINOTE_H_
#define MIDINOTE_H_

#include "score/playable.h"

class CAVoice;

class CAMidiNote: public CAPlayable {
public:
	CAMidiNote( int pitch, int timeStart, int timeLength, CAVoice *v );
	virtual ~CAMidiNote();

	CAMidiNote* clone(CAVoice* voice=0);
	int compare(CAMusElement* elt);

	int midiPitch() { return _midiPitch; }
	void setMidiPitch( int m ) { _midiPitch = m; }

private:
	int _midiPitch;
};

#endif /* MIDINOTE_H_ */
