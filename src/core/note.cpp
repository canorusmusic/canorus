/*
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 *
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#include "core/note.h"
#include "core/voice.h"
#include "core/staff.h"
#include "core/clef.h"

/*!
	\class CANote
	\brief Represents a note in the score.
	
	This class represents every note in the score. It inherits the base class CAPlayable.
*/

/*!
	Creates a new note with playable length \a length in voice \a voice with pitch \a pitch
	and accidentals \a accs, with starting time in the score \a timeStart and number of dots \a dotted.
	timeLength is calculated automatically from the playable length.
	
	\sa CAPlayableLength, CAPlayable, CAVoice, _pitch, _accs
*/
CANote::CANote(CAPlayableLength length, CAVoice *voice, int pitch, signed char accs, int timeStart, int dotted)
 : CAPlayable(length, voice, timeStart, dotted) {
	_musElementType = CAMusElement::Note;
	_forceAccidentals = false;
	_accs = accs;
	_stemDirection = StemPrefered;
	
	_pitch = pitch;
	_midiPitch = CANote::pitchToMidiPitch(pitch, _accs);
	
	calculateNotePosition();
}

/*!
	\enum CANote::CAStemDirection
	\brief Direction of the note's stem
	
	This type represents the direction of the note's stem.
	\todo Stem itself will probably be created as a separate class in the future. This will give us
	possibility for a chord to have a common stem, apply additional stem properties etc. -Matevz
	
	Possible values:
		- StemNeutral
			Up if under the middle line, down if above the middle line.
		- StemUp
			Always up.
		- StemDown
			Always down.
		- StemPrefered
			Use the voice's prefered direction.
*/

CANote *CANote::clone() {
	CANote *d = new CANote(_playableLength, _voice, _pitch, _accs, _timeStart, _dotted);
	
	return d;
}

/*!
	Dependent on the current clef calculates and stores internally the vertical
	note position in the staff.
	
	\sa _notePosition, notePosition()
*/
void CANote::calculateNotePosition() {
	CAClef *clef = (_voice?_voice->getClef(this):0);
	
	_notePosition = _pitch + (clef?clef->c1():-2) - 28;
}

/*!
	Generates the note name on the given pitch \a pitch with accidentals \a accs.
	Note name ranges are from C,, for sub-contra octave to c''''' for fifth octave.
	This method is usually used for showing the note pitch in status bar.
	
	\sa _pitch, _accs
*/
const QString CANote::generateNoteName(int pitch, int accs) {
	QString name;
	
	name = (char)((pitch+2)%7 + 'a');
	while (accs>0) { name += "is"; accs--; }
	while (accs<0) { if (name!="e" && name!="a") name+="es"; else name+="s"; accs++; }
	
	if (pitch < 21)
		name = name.toUpper();
	
	for (int i=0; i<(pitch-21)/7; i++)
		name.append('\'');
	
	if (pitch<14)
		name.append(',');
	if (pitch<7)
		name.append(',');
	
	return name;
}

/*!
	Composes the note pitch name for the CanorusML format.
	
	\deprecated CanorusML was rewritten to a complete XML structure.
	No conversion to human-readable pitches is needed anymore. This
	function could be used for parsing LilyPond for example though. -Matevz
*/
const QString CANote::pitchML() {
	QString name;
	
	name = (char)((_pitch+2)%7 + 'a');
	
	for (int i=0; i < _accs; i++)
		name += "is";	//append as many -is-es as necessary
	
	for (int i=0; i > _accs; i--) {
		if ( ((name == "e") || (name == "a")) && (i==0) )
			name += "s";	//for e and a, only append single -s the first time
		else
			name += "es";	//otherwise, append normally as many es-es as necessary
	}
	
	return name;
}

/*!
	Sets the note pitch to \a pitch.
	
	\sa _pitch
*/
void CANote::setPitch(int pitch) {
	_pitch = pitch;
	_midiPitch = CANote::pitchToMidiPitch(pitch, _accs);
	
	calculateNotePosition();
}

/*!
	Composes the note length for the CanorusML format.
	
	\deprecated CanorusML was rewritten to a complete XML structure.
	No conversion to human-readable lengths is needed anymore. This
	function could be used for parsing LilyPond for example though. -Matevz
*/
const QString CANote::lengthML() {
	QString length;
	switch (_playableLength) {
		case CAPlayable::Breve:
			length = "0";
			break;
		case CAPlayable::Whole:
			length = "1";
			break;
		case CAPlayable::Half:
			length = "2";
			break;
		case CAPlayable::Quarter:
			length = "4";
			break;
		case CAPlayable::Eighth:
			length = "8";
			break;
		case CAPlayable::Sixteenth:
			length = "16";
			break;
		case CAPlayable::ThirtySecond:
			length = "32";
			break;
		case CAPlayable::SixtyFourth:
			length = "64";
			break;
		case CAPlayable::HundredTwentyEighth:
			length = "128";
			break;
	}
	
	return length;
}

/*!
	Returns true, if the note is part of a chord; otherwise false.
*/
bool CANote::isPartOfTheChord() {
	int idx = _voice->indexOf(this);
	
	//is there a note with the same start time after ours?
	if (idx+1<_voice->musElementCount() && _voice->musElementAt(idx+1)->musElementType()==CAMusElement::Note && _voice->musElementAt(idx+1)->timeStart()==_timeStart)
		return true;
	
	//is there a note with the same start time before ours?
	if (idx>0 && _voice->musElementAt(idx-1)->musElementType()==CAMusElement::Note && _voice->musElementAt(idx-1)->timeStart()==_timeStart)
		return true;
	
	return false;
}

/*!
	Returns true, if the note is the first in the list of the chord; otherwise false.
*/
bool CANote::isFirstInTheChord() {
	int idx = _voice->indexOf(this);
	
	//is there a note with the same start time before ours?
	if (idx>0 && _voice->musElementAt(idx-1)->musElementType()==CAMusElement::Note && _voice->musElementAt(idx-1)->timeStart()==_timeStart)
		return false;
	
	return true;
}

/*!
	Returns true, if the note is the last in the list of the chord; otherwise false.
*/
bool CANote::isLastInTheChord() {
	int idx = _voice->indexOf(this);
	
	//is there a note with the same start time after ours?
	if (idx+1<_voice->musElementCount() && _voice->musElementAt(idx+1)->musElementType()==CAMusElement::Note && _voice->musElementAt(idx+1)->timeStart()==_timeStart)
		return false;
	
	return true;
}

/*!
	Finds and returns a list of notes with the same start time - the whole chord - in the current voice.
	Returns a single element in the list - only the note itself, if the note isn't part of the chord.
*/
QList<CANote*> CANote::chord() {
	QList<CANote*> list;
	int idx = _voice->indexOf(this) - 1;
	
	while (idx>=0 &&
	       _voice->musElementAt(idx)->musElementType()==CAMusElement::Note &&
	       _voice->musElementAt(idx)->timeStart()==_timeStart)
		idx--;
	
	for (idx+=1;
	     (idx<_voice->musElementCount()) && (_voice->musElementAt(idx)->musElementType()==CAMusElement::Note) && (_voice->musElementAt(idx)->timeStart()==_timeStart);
	     idx++)
		list << (CANote*)_voice->musElementAt(idx);
	
	return list;
}

int CANote::compare(CAMusElement *elt) {
	if (elt->musElementType()!=CAMusElement::Note)
		return -1;
	
	int diffs=0;
	if ((_pitch!=((CANote*)elt)->pitch()) ||
	(_accs!=((CANote*)elt)->accidentals())) diffs++;
	if (_playableLength!=((CAPlayable*)elt)->playableLength()) diffs++;
	if (_midiPitch!=((CANote*)elt)->midiPitch()) diffs++;
	if (_midiLength!=((CANote*)elt)->midiLength()) diffs++;
	if (timeLength()!=((CANote*)elt)->timeLength()) diffs++;
	
	return diffs;
}

/*!
	Converts stem direction CAStemDirection to QString.
	This is usually used when saving the score.
	
	\sa CAStemDirection, CACanorusML
*/
const QString CANote::stemDirectionToString(CANote::CAStemDirection dir) {
	switch (dir) {
		case CANote::StemUp:
			return "stem-up";
		case CANote::StemDown:
			return "stem-down";
		case CANote::StemNeutral:
			return "stem-neutral";
		case CANote::StemPrefered:
			return "stem-prefered";
		default:
			return "";
	}
}

/*!
	Converts stem direction in QString format to CAStemDirection.
	This is usually used when loading a score.
	
	\sa CAStemDirection, CACanorusML
*/
CANote::CAStemDirection CANote::stemDirectionFromString(const QString dir) {
	if (dir=="stem-up") {
		return CANote::StemUp;
	} else
	if (dir=="stem-down") {
		return CANote::StemDown;
	} else
	if (dir=="stem-neutral") {
		return CANote::StemNeutral;
	} else
	if (dir=="stem-prefered") {
		return CANote::StemPrefered;
	} else
		return CANote::StemPrefered;
}

/*!
	Converts the given internal Canorus \a pitch with accidentals \a acc to
	standard unsigned 7-bit MIDI pitch.
	
	\sa _pitch, midiPitchToPitch()
*/
int CANote::pitchToMidiPitch(int pitch, int acc) {
	float step = (float)12/7;
	
	// +0.3 - rounding factor for 7/12 that exactly underlays every tone in octave
	// +0.5 - casting to int cut-off the decimal part, not round it. In order to round it - add 0.5
	// +12 - our logical pitch starts at Sub-contra C, midi counting starts one octave lower
	return (int)(pitch * step + 0.3 + 0.5 + 12) + acc;
}

/*!
	Converts the given standard unsigned 7-bit MIDI pitch to internal Canorus pitch.
	
	\todo This method currently doesn't do anything. Problem is determination of sharp/flat from MIDI. -Matevz
	
	\sa _pitch, pitchToMidiPitch()
*/
int CANote::midiPitchToPitch(int midiPitch) {
	return 0;
}

/*!
	\fn CANote::accidentals()
	Returns number and type of accidentals of the note.
	
	\sa _accs, setAccidentals()
*/

/*!
	\fn CANote::setAccidentals(int accs)
	Sets a number and type of accidentals to \a accs of the note.
	
	\sa _accs, accidentals()
*/

/*!
	\var CANote::_accs
	Number of note accidentals:
		- 0 - neutral
		- 1 - sharp
		- -1 - flat etc.
	
	\sa accidentals(), setAccidentals()
*/

/*!
	\fn CANote::pitch()
	Returns the note pitch in internal Canorus units.
	
	\sa _pitch, setPitch()
*/

/*!
	\fn CANote::setPitch(int pitch)
	Sets the note pitch to \a pitch.
	
	\sa _pitch, pitch()
*/

/*!
	\var CANote::_pitch
	Pitch in internal Canorus format.
		- 0 - sub-contra C
		- 1 - sub-contra D
		- 28 - middle C
		- 56 - c5 etc.
	
	\sa pitch(), setPitch(), midiPitchToPitch(), pitchToMidiPitch(), pitchToString(), generateNoteName()
*/

/*!
	\var CANote::_stemDirection
	Direction of the note's stem, if any.
	
	\sa CAStemDirection
*/

/*!
	\var CANote::_notePosition
	Note location in the staff:
		- 0 - first line
		- 1 - first space
		- -2 - first ledger line below the staff etc.
	
	\sa calculateNotePosition()
*/

/*!
	\var CANote::_forceAccidentals
	Always draw notes accidentals.
	
	\sa _accs
*/
