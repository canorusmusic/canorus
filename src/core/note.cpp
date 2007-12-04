/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
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
	_stemDirection = StemPreferred;
	
	_pitch = pitch;
	_midiPitch = CANote::pitchToMidiPitch(pitch, _accs);
	
	setTieStart( 0 );
	setSlurStart( 0 );
	setPhrasingSlurStart( 0 );
	setTieEnd( 0 );
	setSlurEnd( 0 );
	setPhrasingSlurEnd( 0 );

	calculateNotePosition();
}

CANote::~CANote() {
	if ( tieStart() ) delete tieStart();      // slurs destructor also disconnects itself from the notes
	if ( tieEnd() ) tieEnd()->setNoteEnd( 0 );
	// other slurs are removed or moved in CAVoice::removeElement()
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
		- StemPreferred
			Use the voice's preferred direction.
*/

/*!
	Clones the note with same pitch, voice, timeStart and other properties.
	Does *not* create clones of ties, slurs and phrasing slurs!
*/
CANote *CANote::clone( CAVoice *voice ) {
	CANote *d = new CANote(_playableLength, voice, _pitch, _accs, _timeStart, _dotted);
	d->setStemDirection( stemDirection() );
	
	return d;
}

/*!
	Dependent on the current clef calculates and stores internally the vertical
	note position in the staff.
	
	\sa _notePosition, notePosition()
*/
void CANote::calculateNotePosition() {
	CAClef *clef = (voice()?voice()->getClef(this):0);
	
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
	Sets the note pitch to \a pitch.
	
	\sa _pitch
*/
void CANote::setPitch(int pitch) {
	_pitch = pitch;
	_midiPitch = CANote::pitchToMidiPitch(pitch, _accs);
	
	calculateNotePosition();
	updateTies();
}

/*!
	Returns true, if the note is part of a chord; otherwise false.
*/
bool CANote::isPartOfTheChord() {
	int idx = voice()->indexOf(this);
	
	// is there a note with the same start time after ours?
	if (idx+1<voice()->musElementCount() && voice()->musElementAt(idx+1)->musElementType()==CAMusElement::Note && voice()->musElementAt(idx+1)->timeStart()==_timeStart)
		return true;
	
	// is there a note with the same start time before ours?
	if (idx>0 && voice()->musElementAt(idx-1)->musElementType()==CAMusElement::Note && voice()->musElementAt(idx-1)->timeStart()==_timeStart)
		return true;
	
	return false;
}

/*!
	Returns true, if the note is the first in the list of the chord; otherwise false.
*/
bool CANote::isFirstInTheChord() {
	int idx = voice()->indexOf(this);
	
	//is there a note with the same start time before ours?
	if (idx>0 && voice()->musElementAt(idx-1)->musElementType()==CAMusElement::Note && voice()->musElementAt(idx-1)->timeStart()==_timeStart)
		return false;
	
	return true;
}

/*!
	Returns true, if the note is the last in the list of the chord; otherwise false.
*/
bool CANote::isLastInTheChord() {
	int idx = voice()->indexOf(this);
	
	//is there a note with the same start time after ours?
	if (idx+1<voice()->musElementCount() && voice()->musElementAt(idx+1)->musElementType()==CAMusElement::Note && voice()->musElementAt(idx+1)->timeStart()==_timeStart)
		return false;
	
	return true;
}

/*!
	Generates a list of notes with the same start time - the whole chord - in the current voice.
	Notes in chord keep the order present in the voice. This is usually bottom-up.
	
	Returns a single element in the list - only the note itself, if the note isn't part of the chord.
	
	\sa CAVoice::addNoteToChord()
*/
QList<CANote*> CANote::getChord() {
	QList<CANote*> list;
	int idx = voice()->indexOf(this) - 1;
	
	while (idx>=0 &&
	       voice()->musElementAt(idx)->musElementType()==CAMusElement::Note &&
	       voice()->musElementAt(idx)->timeStart()==timeStart())
		idx--;
	
	for (idx++;
	     (idx<voice()->musElementCount()) && (voice()->musElementAt(idx)->musElementType()==CAMusElement::Note) && (voice()->musElementAt(idx)->timeStart()==timeStart());
	     idx++)
		list << static_cast<CANote*>(voice()->musElementAt(idx));
	
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
	if (timeLength()!=((CANote*)elt)->timeLength()) diffs++;
	
	return diffs;
}

/*!
	Sets the stem direction and update tie, slur and phrasing slur direction.
*/
void CANote::setStemDirection( CAStemDirection dir ) {
	_stemDirection = dir;
}

/*!
	Looks at the tieStart() and tieEnd() ties and unties the note and tie if the
	previous/next note pitch differs.
*/
void CANote::updateTies() {
	// break the tie, if needed
	if ( tieStart() && tieStart()->noteEnd() &&
	     pitch()!=tieStart()->noteEnd()->pitch() ) {
		// break the tie, if the first note isn't the same pitch
		tieStart()->noteEnd()->setTieEnd( 0 );
		tieStart()->setNoteEnd( 0 );
	}
	if ( tieEnd() && tieEnd()->noteStart() &&
	     pitch()!=tieEnd()->noteStart()->pitch() ) {
		// break the tie, if the next note isn't the same pitch
		tieEnd()->setNoteEnd( 0 );
		setTieEnd( 0 );
	}
	
	// fix/create a tie, if needed
	QList<CANote*> noteList;
	if (voice()) noteList = voice()->getNoteList();
	
	// checks a tie of the potential left note
	CANote *leftNote = 0;
	for (int i=0; i<noteList.count() && noteList[i]->timeEnd()<=timeStart(); i++) { // get the left note
		if ( noteList[i]->timeEnd()==timeStart() && noteList[i]->pitch()==pitch() ) {
			leftNote = noteList[i];
			break;
		}
	}
	if ( leftNote && leftNote->tieStart() ) {
		leftNote->tieStart()->setNoteEnd( this );
		setTieEnd( leftNote->tieStart() );
	}
	
	// checks a tie of the potential right note
	CANote *rightNote = 0;
	for (int i=0; i<noteList.count() && noteList[i]->timeStart()<=timeEnd(); i++) { // get the right note
		if ( noteList[i]->timeStart()==timeEnd() && noteList[i]->pitch()==pitch() ) {
			rightNote = noteList[i];
			break;
		}
	}
	if ( rightNote && tieStart() ) {
		rightNote->setTieEnd( tieStart() );
		tieStart()->setNoteEnd( rightNote );
	}	
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
		case CANote::StemPreferred:
			return "stem-preferred";
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
	if (dir=="stem-preferred") {
		return CANote::StemPreferred;
	} else
		return CANote::StemPreferred;
}

/*!
	Converts the given internal Canorus \a pitch with accidentals \a acc to
	standard unsigned 7-bit MIDI pitch.
	
	\sa _pitch, midiPitchToPitch()
*/
int CANote::pitchToMidiPitch(int pitch, int acc) {
	float step = (float)12/7;
	
	// +0.3 - rounding factor for 7/12 that exactly underlays every tone in octave, if rounded
	// +12 - our logical pitch starts at Sub-contra C, midi counting starts one octave lower
	return qRound(pitch*step + 0.3 + 12) + acc;
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
	Returns the actual stem direction (the one which is drawn). Always returns stem up or stem down.
*/
CANote::CAStemDirection CANote::actualStemDirection() {
	switch ( stemDirection() ) {
		case StemUp:
		case StemDown:
			return stemDirection();
			break;
		
		case StemNeutral:
			if ( staff() && notePosition() < staff()->numberOfLines()-1 )	// position from 0 to half of the number of lines - where position has step of 2 per line
				return StemUp;
			else
				return StemDown;
			break;
		
		case StemPreferred:
			if (!voice()) { return StemUp; }
			
			switch ( voice()->stemDirection() ) {
				case StemUp:
				case StemDown:
					return voice()->stemDirection();
					break;
				
				case StemNeutral:
					if ( staff() && notePosition() < staff()->numberOfLines()-1 )	// position from 0 to half of the number of lines - where position has step of 2 per line
						return StemUp;
					else
						return StemDown;
					break;
			}
			break;
	}
}

/*!
	Determines the right slur direction of the note.
	Slur should be on the other side of the stem, if the stem direction is neutral
	or on the same side if the stem direction is set strictly to up and down (or preferred).
*/
CASlur::CASlurDirection CANote::actualSlurDirection() {
	CAStemDirection dir = actualStemDirection();
	
	if ( stemDirection()==StemNeutral || (stemDirection()==StemPreferred && voice() && voice()->stemDirection()==StemNeutral) ) {
		if (dir==StemUp) return CASlur::SlurDown;
		else return CASlur::SlurUp;
	} else {
		if (dir==StemUp) return CASlur::SlurUp;
		else return CASlur::SlurDown;
	}
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
