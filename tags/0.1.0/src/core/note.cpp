/** @file core/note.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "core/note.h"
#include "core/voice.h"
#include "core/staff.h"
#include "core/clef.h"

CANote::CANote(CAPlayableLength length, CAVoice *voice, int pitch, signed char accs, int timeStart, int dotted)
 : CAPlayable(length, voice, timeStart, dotted) {
	_musElementType = CAMusElement::Note;
	_forceAccidentals = false;
	_accs = accs;

	_pitch = pitch;
	_midiPitch = CAPlayable::pitchToMidiPitch(pitch, _accs);

	calculateNotePosition();
}

CANote *CANote::clone() {
	CANote *d = new CANote(_playableLength, _voice, _pitch, _accs, _timeStart, _dotted);
	
	return d;
}

void CANote::calculateNotePosition() {
	CAClef *clef = (_voice?_voice->getClef(this):0);
	
	_notePosition = _pitch + (clef?clef->c1():-2) - 28;
}

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
void CANote::setPitch(int pitch) {
	_pitch = pitch;
	_midiPitch = CAPlayable::pitchToMidiPitch(pitch, _accs);

	calculateNotePosition();
}

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

bool CANote::isFirstInTheChord() {
	int idx = _voice->indexOf(this);
	
	//is there a note with the same start time before ours?
	if (idx>0 && _voice->musElementAt(idx-1)->musElementType()==CAMusElement::Note && _voice->musElementAt(idx-1)->timeStart()==_timeStart)
		return false;
	
	return true;
}

bool CANote::isLastInTheChord() {
	int idx = _voice->indexOf(this);
	
	//is there a note with the same start time after ours?
	if (idx+1<_voice->musElementCount() && _voice->musElementAt(idx+1)->musElementType()==CAMusElement::Note && _voice->musElementAt(idx+1)->timeStart()==_timeStart)
		return false;
	
	return true;
}

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
