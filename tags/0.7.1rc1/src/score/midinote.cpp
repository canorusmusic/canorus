/*!
	Copyright (c) 2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/midinote.h"

/*!
	\class CAMidiNote
	\brief Represents a note with custom length and Midi-defined pitch

	This class is usually used when a composer wants to import his improvisations
	and the input Midi files are naturally recorded and are too difficult to quantize.
	However, composer wants to see his recorded notes in the staff so he can transcribe
	the prototype improvisation into actual notes.
	The CAMidiNote class represents a Midi Note-On/Note-Off pair of events with defined
	timeStart, timeLength and Midi pitch. The class is usually used to graphically
	display the recorded Midi events in the staff and nothing more.
*/

CAMidiNote::CAMidiNote( int midiPitch, int timeStart, int timeLength, CAVoice *voice )
 : CAPlayable( CAPlayableLength::Undefined, voice, timeStart, timeLength ),
   _midiPitch(midiPitch) {
	setMusElementType( MidiNote );
}

CAMidiNote::~CAMidiNote() {
}

CAMidiNote *CAMidiNote::clone( CAVoice *v ) {
	return new CAMidiNote( midiPitch(), timeStart(), timeLength(), v );
}

int CAMidiNote::compare( CAMusElement *elt ) {
	if (elt->musElementType()!=CAMusElement::MidiNote) {
		return -1;
	} else
	if (static_cast<CAMidiNote*>(elt)->midiPitch()!=midiPitch()) {
		return 1;
	} else {
		return 0;
	}
}
