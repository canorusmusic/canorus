/*
 * Copyright (c) 2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 *
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#include <QRegExp>
#include <QList>

#include "export/lilypondexport.h"

#include "core/staff.h"
#include "core/voice.h"

/*!
	Exports the given voice music elements to LilyPond syntax.
	
	\sa CALilypondImport
*/
void CALilyPondExport::exportVoice(CAVoice *v) {
	setCurVoice(v);
	
	int lastNotePitch;   // initialized by writeRelativeIntro()
	int firstNotePitchInChord;
	int curStreamTime = 0;
	CAPlayable::CAPlayableLength lastPlayableLength;
	int lastPlayableDotted=0;
	
	// Write \relative note for the first note
	lastNotePitch = writeRelativeIntro();
	
	// start of the voice block
	out() << "{ ";
	
	for (int i=0; i<v->musElementCount(); i++, out() << " ") { // append blank after each element
		// (CAMusElement)
		switch (v->musElementAt(i)->musElementType()) {
			case CAMusElement::Clef: {
				// CAClef
				CAClef *clef = static_cast<CAClef*>(v->musElementAt(i));
				if (clef->timeStart()!=curStreamTime) break;	//! \todo If the time isn't the same, insert hidden rests to fill the needed time
				out() << "\\clef \"";
				out() << clefTypeToLilyPond(clef->clefType());
				out() << "\"";
				
				break;
			}
			case CAMusElement::KeySignature: {
				// CAKeySignature
				CAKeySignature *key = static_cast<CAKeySignature*>(v->musElementAt(i));
				if (key->timeStart()!=curStreamTime) break;	//! \todo If the time isn't the same, insert hidden rests to fill the needed time
				out() << "\\key "
				    << keySignaturePitchToLilyPond(key->numberOfAccidentals(), key->majorMinorGender()) << " "
				    << keySignatureGenderToLilyPond(key->majorMinorGender());
				
				break;
			}
			case CAMusElement::TimeSignature: {
				// CATimeSignature
				CATimeSignature *time = static_cast<CATimeSignature*>(v->musElementAt(i));
				if (time->timeStart()!=curStreamTime) break;	//! \todo If the time isn't the same, insert hidden rests to fill the needed time
				out() << "\\time " << time->beats() << "/" << time->beat();
				
				break;
			}
			case CAMusElement::Barline: {
				// CABarline
				CABarline *bar = static_cast<CABarline*>(v->musElementAt(i));
				if (bar->timeStart()!=curStreamTime) break;	//! \todo If the time isn't the same, insert hidden rests to fill the needed time
				if (bar->barlineType() == CABarline::Single)
					out() << "|";
				
				break;
			}
			case CAMusElement::Note: {
				// CANote
				CANote *note = static_cast<CANote*>(v->musElementAt(i));
				if (note->timeStart()!=curStreamTime) break;	//! \todo If the time isn't the same, insert hidden rests to fill the needed time
				if (note->isPartOfTheChord() && note->isFirstInTheChord()) {
					out() << "<";
					firstNotePitchInChord=note->pitch();
				}
				
				// write the note name
				out() << relativePitchToString(note->pitch(), note->accidentals(), lastNotePitch);
				
				if (lastPlayableLength != note->playableLength() || lastPlayableDotted != note->dotted()) {
					out() << playableLengthToLilyPond(note->playableLength(), note->dotted());
				}
				
				lastNotePitch = note->pitch();
				lastPlayableLength = note->playableLength();
				lastPlayableDotted = note->dotted();
				
				// finish the chord stanza if that's the last note of the chord
				if (note->isPartOfTheChord() && note->isLastInTheChord()) {
					out() << ">";
					lastNotePitch = firstNotePitchInChord;
				}
				
				// add to the stream time, if the note is not part of the chord or is the last one in the chord
				if (!note->isPartOfTheChord() ||
				    (note->isPartOfTheChord() && note->isLastInTheChord()) )
					curStreamTime += note->timeLength();
				
				break;
			}
			case CAMusElement::Rest: {
				// CARest
				CARest *rest = (CARest*)v->musElementAt(i);
				if (rest->timeStart()!=curStreamTime) break;	//! \todo If the time isn't the same, insert hidden rests to fill the needed time
				
				out() << restTypeToLilyPond(rest->restType());
				
				if (lastPlayableLength!=rest->playableLength() || lastPlayableDotted!=rest->dotted()) {
					out() << playableLengthToLilyPond(rest->playableLength(), rest->dotted());
				}
				
				lastPlayableLength = rest->playableLength();
				lastPlayableDotted = rest->dotted();
				curStreamTime += rest->timeLength();
				
				break;
			}
		}
	}
	
	// end of the voice block
	out() << "}";
}

/*!
	Constructor for voice export. Called when viewing a single voice source.
	Exports a voice to LilyPond syntax using the given text stream.
*/
CALilyPondExport::CALilyPondExport(CAVoice *voice, QTextStream *out) {
	_out = out;
	exportVoice(voice);
}

/*!
	Writes the voice's \relative note intro and returns the note pitch for the current voice.
	This function is usually used for writing the beginning of the voice.
	\warning This function doesn't write "{" paranthesis to mark the start of the voice!
*/
int CALilyPondExport::writeRelativeIntro() {
	int i;
	
	// find the first playable element and set the key signature if found any 
	for ( i=0;
	      (i<curVoice()->musElementCount() &&
	      (curVoice()->musElementAt(i)->musElementType()!=CAMusElement::Note));
	      i++);
	
	// no playable elements present, return default c' pitch
	if (i==curVoice()->musElementCount())
		return 28;
	
	int notePitch = static_cast<CANote*>(curVoice()->musElementAt(i))->pitch();
	int cPitch = ((notePitch + 3) / 7) * 7;
	out() << "\\relative "
	      << relativePitchToString(cPitch, 0, 28) << " "; // LilyPond default C is c1
	
	return cPitch;
}

/*!
	This function returns the relative note pitch in LilyPond syntax on the given
	Canorus pitch and previous note pitch.
	
	eg. pitch=10, accs=1, prevPitch=5 => returns "f'"
	
	\sa notePitchToLilyPond()
*/
const QString CALilyPondExport::relativePitchToString(int pitch, signed char accs, int prevPitch) {
	// write the note name
	QString stringPitch = notePitchToLilyPond(pitch, accs);
	
	// write , or ' to lower/higher a note
	int delta = prevPitch - pitch;
	while (delta > 3) { // add the needed amount of the commas
		stringPitch += ",";
		delta -= 7;
	}
	while (delta < -3) { // add the needed amount of the apostrophes
		stringPitch += "'";
		delta += 7;
	}
	
	return stringPitch;
}


/*!
	Converts the given clefType to LilyPond syntax.
*/
const QString CALilyPondExport::clefTypeToLilyPond(CAClef::CAClefType clefType) {
	QString type;
	
	switch (clefType) {
		case CAClef::Treble:
			type = "treble";
			break;
		case CAClef::Bass:
			type = "bass";
			break;
		case CAClef::Alto:
			type = "alto";
			break;
		case CAClef::Tenor:
			type = "tenor";
			break;
		case CAClef::Soprano:
			type = "soprano";
			break;
	}
	
	return type;
}

/*!
	Returns the key signature pitch in LilyPond syntax.
	Pitch is always lower-case regardless of the Key signature gender.
	eg. "e" for E-Major (4 sharps) or e-minor (1 sharp)
	
	\sa keySignatureGenderToLilyPond()
*/
const QString CALilyPondExport::keySignaturePitchToLilyPond(signed char numberOfAccs, CAKeySignature::CAMajorMinorGender gender) {
	// calculate key signature pitch from number of accidentals
	int pitch = ((4*numberOfAccs) % 7) + ((numberOfAccs < 0) ? 7 : 0);
	
	if (gender==CAKeySignature::Minor)	// find the parallel minor key
		pitch = (pitch + 5) % 7;
	
	signed char accs = 0;
	
	if (numberOfAccs>5)
		accs = 1;
	else if (numberOfAccs<-1)
		accs = -1;
	
	return notePitchToLilyPond(pitch, accs);
}

/*!
	Returns the key signature gender major/minor in LilyPond syntax.
	
	\sa keySignaturePitchToLilyPond()
*/
const QString CALilyPondExport::keySignatureGenderToLilyPond(CAKeySignature::CAMajorMinorGender gender) {
	switch (gender) {
		case CAKeySignature::Major: return QString("\\major");
		case CAKeySignature::Minor: return QString("\\minor");
	}
	
	return QString();
}

/*!
	Converts the note length to LilyPond syntax.
*/
const QString CALilyPondExport::playableLengthToLilyPond(CAPlayable::CAPlayableLength playableLength, int dotted) {
	QString length;
	switch (playableLength) {
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
	
	for (int j=0; j<dotted; j++) length += ".";
	
	return length;
}

/*!
	Converts the note pitch to LilyPond syntax.
*/
const QString CALilyPondExport::notePitchToLilyPond(int pitch, signed char accs) {
	QString name;
	
	name = (char)((pitch+2)%7 + 'a');
	
	for (int i=0; i < accs; i++)
		name += "is";	// append as many -is-es as necessary
	
	for (int i=0; i > accs; i--) {
		if ( (name == "e") || (name == "a") )
			name += "s";	// for pitches E and A, only append single -s the first time
		else if (name[0]=='a')
			name += "as";	// for pitch A, append -as instead of -es
		else
			name += "es";	// otherwise, append normally as many es-es as necessary
	}
	
	return name;
}

/*!
	Converts the rest type to LilyPond syntax.
*/
const QString CALilyPondExport::restTypeToLilyPond(CARest::CARestType type) {
	switch (type) {
		case CARest::Normal:
			return "r";
			break;
		case CARest::Hidden:
			return "s";
			break;
	}
}

/*!
	Reads voice music elements in LilyPond syntax.
	The given voice should already be created, (optionally) cleared and assigned to a staff.
	
	\sa exportVoice()
*/
