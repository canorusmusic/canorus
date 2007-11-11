/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/muselement.h"
#include "core/voice.h"
#include "core/staff.h"
#include "core/clef.h"
#include "core/note.h"
#include "core/rest.h"
#include "core/playable.h"
#include "core/lyricscontext.h"
#include "core/slur.h"
#include "interface/mididevice.h"

/*!
	\class CAVoice
	\brief Class which represents a voice in the staff.
	
	CAVoice is a class which holds music elements in the staff. In hieararchy, staff
	includes multiple voices and every voice includes multiple music elements.
	
	\sa CAStaff, CAMusElement
*/

/*!
	Creates a new voice named \a name, in \a staff, \a voiceNumber and \a stemDirection of notes stems.
	Voice number starts at 1.
*/
CAVoice::CAVoice( const QString name, CAStaff *staff, CANote::CAStemDirection stemDirection, int voiceNumber ) {
	_staff = staff;
	_name = name;
	
	if ( !voiceNumber && staff ) {
		_voiceNumber = staff->voiceCount()+1;
	} else {
		_voiceNumber = voiceNumber;
	}
	_stemDirection = stemDirection;
	
	_midiChannel = (staff ? CAMidiDevice::freeMidiChannel( staff->sheet() ) : 0);
	_midiProgram = 0;
}

/*!
	Clears and destroys the current voice.
	This also destroys all non-shared music elements held by the voice.
	
	\sa clear()
*/
CAVoice::~CAVoice() {
	clear();
	
	for (int i=0; i<lyricsContextList().size(); i++)
		lyricsContextList().at(i)->setAssociatedVoice( 0 );
	
	if (staff())
		staff()->removeVoice(this);
}

/*!
	Clones the current voice including all the music elements.
	Sets the voice staff to \a newStaff. If none given, use the original staff.
*/
CAVoice *CAVoice::clone( CAStaff* newStaff ) {
	CAVoice *newVoice = new CAVoice( name(), newStaff );
	newVoice->cloneVoiceProperties( this );
	newVoice->setStaff( newStaff );
	
	return newVoice;
}

/*!
	Sets the properties of the given voice to this voice.
*/
void CAVoice::cloneVoiceProperties( CAVoice *voice ) {
	setName( voice->name() );
	setStaff( voice->staff() );
	setVoiceNumber( voice->voiceNumber() );
	setStemDirection( voice->stemDirection() );
	setMidiChannel( voice->midiChannel() );
	setMidiProgram( voice->midiProgram() );
	setLyricsContexts( voice->lyricsContextList() );
}

/*!
	Clones the voice.
	This method is provided for convenience.
*/
CAVoice *CAVoice::clone() {
	return clone( staff() );
}

/*!
	Destroys all non-shared music elements held by the voice.
	
	When clearing the whole staff, make sure the voice is *deleted*.
	It is automatically removed from the staff.
*/
void CAVoice::clear() {
	while ( _musElementList.size() ) {
		// deletes an element only if it's not present in other voices or we're deleting the last voice
		if ( _musElementList.front()->isPlayable() || staff() && staff()->voiceCount()<2 )
			delete _musElementList.front(); // CAMusElement's destructor removes it from the list
		else
			_musElementList.removeFirst();
	}
}

/*!
	Inserts the music element \a elt after the last element which startTime is lesser
	or equal to the given \a elt one. Updates start times for elements after it, if
	\a updateT is set. This is false when adding a note to a chord; otherwise true.
	If such an element doesn't exist, appends a music element.
	
	\warning If you're inserting shared signs like barlines, clefs, time signatures and other
	non-playable signs use this method indirectly by calling CAStaff::insertSign(), because
	the mentioned signs MUST be present in *all* voices. Use manually this method only if you
	know what you're doing or you're inserting playable non-shared elements.
	
	\sa insertMusElementBefore(), insertMusElementAfter()
*/
void CAVoice::insertMusElement(CAMusElement *elt, bool updateT) {
	int i;
	for (i=0;
	     (i < _musElementList.size()) && ((_musElementList[i]->timeStart() < elt->timeStart() ||
	      (_musElementList[i]->timeStart() == elt->timeStart() && !_musElementList[i]->isPlayable())));
	     i++);
	
	_musElementList.insert(i, elt);
	
	if (updateT)
		updateTimes(i, true);
}

/*!
	Adds a \a note to an already existing \a referenceNote chord or a single note and
	creates a chord out of it.
	Notes in a chord always needs to be sorted by the pitch rising.
	Chord in Canorus isn't its own structure but simply a list of notes sharing the
	same start time.
	
	Returns true, if a referenceNote was found and a note was added; otherwise false.
	
	\sa CANote::chord()
*/
bool CAVoice::addNoteToChord(CANote *note, CANote *referenceNote) {
	int idx = _musElementList.indexOf(referenceNote);
	
	if (idx==-1)
		return false;
	
	QList<CANote*> chord = referenceNote->chord();
	idx = _musElementList.indexOf(chord.first());
	
	int i;
	for (i=0; i<chord.size() && chord[i]->pitch() < note->pitch(); i++);
	
	_musElementList.insert(idx+i, note);
	
	return true;
}

/*!
	Inserts the \a elt right before the given \a eltAfter. If \a eltAfter is 0, it
	appends the element. It updates start times of elements after the given element if
	\a updateTimes is true. If \a force is set, it finds the nearest element with
	larger start time of the given element and inserts the given element before it.
	
	\warning If you're inserting shared signs like barlines, clefs, time signatures and other
	non-playable signs use this method indirectly by calling CAStaff::insertSign(), because
	the mentioned signs MUST be present in *all* voices. Use manually this method only if you
	know what you're doing or you're inserting playable non-shared elements.
	
	Returns true, if \a eltAfter was found and the elt was inserted/appended; otherwise
	false.
*/
bool CAVoice::insertMusElementBefore(CAMusElement *elt, CAMusElement *eltAfter, bool updateT, bool force) {
	if (!eltAfter || !_musElementList.size()) {
		_musElementList << elt;
		return true;
	}
	
	int i;
	for (i=0; (i<_musElementList.size()) && (_musElementList[i] != eltAfter); i++);
	
	// if element wasn't found and the element before is slur
	if ( eltAfter->musElementType()==CAMusElement::Slur && i==_musElementList.size() )
		for (i=0; (i<_musElementList.size() && _musElementList[i] != static_cast<CASlur*>(eltAfter)->noteStart()); i++);
	
	if (i==_musElementList.size())
		if (!force)
			return false;
		else {
			int endTime = eltAfter->timeEnd();
			for (i=0; (i<_musElementList.size()) && (_musElementList[i]->timeStart() < endTime); i++);
			_musElementList.insert(i, elt);
	} else {
		if (_musElementList[i]->musElementType()==CAMusElement::Note &&	// a small check to see if a user wanted to insert an element after a chord, but selected a note in the middle of a chord (not the lowest one)
		    ((CANote*)_musElementList[i])->isPartOfTheChord() &&
		    eltAfter->timeStart()!=elt->timeStart() ) {
			i = _musElementList.indexOf(((CANote*)_musElementList[i])->chord().front());
		}
		_musElementList.insert(i, elt);
	}
	
	if (updateT)
		updateTimes(i, true);

	return true;
}

/*!
	Inserts the \a elt right after the given \a eltBefore. If \a eltBefore is 0, it
	prepends the element. It updates start times of elements after the given element if
	\a updateTimes is true. If \a force is set, it finds the nearest element with
	smaller start time of the given element and inserts the given element after it.
	
	\warning If you're inserting shared signs like barlines, clefs, time signatures and other
	non-playable signs use this method indirectly by calling CAStaff::insertSign(), because
	the mentioned signs MUST be present in *all* voices. Use manually this method only if you
	know what you're doing or you're inserting playable non-shared elements.
	
	Returns true, if \a eltBefore was found and the elt was inserted/appended;
	otherwise false.
*/
bool CAVoice::insertMusElementAfter(CAMusElement *elt, CAMusElement *eltBefore, bool updateT, bool force) {
	if (!eltBefore || !_musElementList.size()) {
		_musElementList.push_front(elt);
		if (updateT)
			updateTimes(0, true);
		return true;
	}
	
	int i;
	for (i=0; (i<_musElementList.size()) && (_musElementList[i] != eltBefore); i++);
	
	// if element wasn't found and the element before is slur
	if ( eltBefore->musElementType()==CAMusElement::Slur && i==_musElementList.size() )
		for (i=0; (i<_musElementList.size() && _musElementList[i] != static_cast<CASlur*>(eltBefore)->noteStart()); i++);
	
	if (i==_musElementList.size()) {
		if (!force)
			return false;
		else {
			int endTime = eltBefore->timeEnd();
			for (i=0; (i<_musElementList.size()) && (_musElementList[i]->timeStart() < endTime); i++);
			_musElementList.insert(i, elt);
		}
	} else {
		if (_musElementList[i]->musElementType()==CAMusElement::Note &&	//a small check to see if a user wanted to insert an element after a chord, but selected a note in the middle of a chord (not the lowest one)
		    ((CANote*)_musElementList[i])->isPartOfTheChord() &&
		    eltBefore->timeStart()!=elt->timeStart() ) {
			i = _musElementList.indexOf(((CANote*)_musElementList[i])->chord().back());
		}
		
		_musElementList.insert(++i, elt);
	}
	
	if (updateT)
		updateTimes(i, true);
	
	return true;
}

/*!
	Returns a pointer to the clef which the given \a elt belongs to.
	Returns 0, if no clefs placed yet.
*/
CAClef* CAVoice::getClef(CAMusElement *elt) {
	CAClef* lastClef = 0;
	int i;
	for (i=0; (i<_musElementList.size()) && (_musElementList[i] != elt); i++) {
		if (_musElementList[i]->musElementType() == CAMusElement::Clef)
			lastClef = (CAClef*)_musElementList[i];
	}
	
	return lastClef;
}

/*!
	If the given \a length is 0, take the given music elements \a elt length and
	updates start times of music elements after the given element.
	
	If the given \a length is not 0, updates start time of the given music element \a
	elt and start times of music elements after it for the given length.
	
	Updating times means increasing start time of music elements for the given length.
	
	This function is usually called when an element is deleted or inserted and
	startTimes after it need to be updated.
	
	Returns true, if \a elt was found and update was made; otherwise false;
	
	\sa updateTimesAfter()
*/
bool CAVoice::updateTimes(CAMusElement *elt, bool nonPlayable, int length) {
	int idx;
	if ((idx = _musElementList.indexOf(elt))!=-1) {
		updateTimes(idx, nonPlayable, length);
		return true;
	}
		return false;
}

/*!
	Increase start times of music elements after the given \a elt for the \a length.
	
	This function is usually called when an element is deleted or inserted and
	startTimes after it need to be updated.
	
	Returns true, if \a elt was found and update was made; otherwise false;
	
	\sa updateTimesAfter()
*/
bool CAVoice::updateTimesAfter(CAMusElement *elt, bool nonPlayable, int length) {
	int idx;
	if ((idx = _musElementList.indexOf(elt))!=-1) {
		updateTimes(idx+1, nonPlayable, length);
		return true;
	}
		return false;
}

/*!
	Updates the musElements timeStarts from index \a idx and on by increasing their
	start times by the \a givenLength.
	
	This function is usually called when an element is deleted or inserted and
	startTimes after it need to be updated.
	
	If \a givenLength is not set, it takes the length of the element at \a idx and
	updates times after it.
*/
void CAVoice::updateTimes(int idx, bool nonPlayable, int givenLength) {
	if (idx >= _musElementList.size())
		return;
	
	int length;
	if (!givenLength)
		length = _musElementList[idx++]->timeLength();
	else
		length = givenLength;
	
	// indent all the music elements after the given index's one.
	// If the music elements aren't connected (previous's end time != next start time)
	for (int i=idx; i<_musElementList.size() && (givenLength ||
	                (_musElementList[i-1]->timeEnd()==_musElementList[i]->timeStart()+length || _musElementList[i-1]->timeStart()==_musElementList[i]->timeStart()+length))
	     ; i++) {
		if (nonPlayable || !nonPlayable && _musElementList[i]->isPlayable())
			_musElementList[i]->setTimeStart(_musElementList[i]->timeStart() + length);
	}
}

/*!
	Removes the given music element \a elt from the voice.
	\warning This function doesn't destroy the object, but only removes its
	reference in the voice.
	
	Returns true, if the element was found and removed; otherwise false.
*/
bool CAVoice::removeElement(CAMusElement *elt) {
	int idx;
	if ((idx = _musElementList.indexOf(elt)) != -1) {	// if the search element is found
		int length = _musElementList[idx]->timeLength();
		
		if (elt->musElementType()==CAMusElement::Note) {
			CANote *n = static_cast<CANote*>(elt);
			if (n->isPartOfTheChord() && n->isFirstInTheChord()) {
				CANote *prevNote = n->chord().at(1);
				prevNote->setSlurStart( n->slurStart() );
				prevNote->setSlurEnd( n->slurEnd() );
				prevNote->setPhrasingSlurStart( n->phrasingSlurStart() );
				prevNote->setPhrasingSlurEnd( n->phrasingSlurEnd() );
			} else if (!(n->isPartOfTheChord())) {
				if ( n->slurStart() ) delete n->slurStart();
				if ( n->slurEnd() ) delete n->slurEnd();
				if ( n->phrasingSlurStart() ) delete n->phrasingSlurStart();
				if ( n->phrasingSlurEnd() ) delete n->phrasingSlurEnd();
			}
		}
		
		_musElementList.removeAt(idx);					// removes the element from the music element list
		return true;
	} else {
		return false;
	}
}

/*!
	Returns the pitch of the last note in the voice (default) or of the first note in
	the last chord, if \a inChord is true.
	
	This method is usually used by LilyPond parser when exporting the document, where
	, or ' octave markings need to be determined.
	
	\sa lastPlayableElt()
*/
int CAVoice::lastNotePitch(bool inChord) {
	for (int i=_musElementList.size()-1; i>=0; i--) {
		if (_musElementList[i]->musElementType()==CAMusElement::Note) {
			if (!((CANote*)_musElementList[i])->isPartOfTheChord() || !inChord)	// the note is not part of the chord
				return (((CANote*)_musElementList[i])->pitch());
			else {
				int chordTimeStart = _musElementList[i]->timeStart();
				int j;
				for (j=i;
				     (j>=0 && _musElementList[j]->musElementType()==CAMusElement::Note && _musElementList[j]->timeStart()==chordTimeStart);
				     j--);
				
				return (((CANote*)_musElementList[j+1])->pitch());
			}
			
		}
		else if (_musElementList[i]->musElementType()==CAMusElement::Clef)
			return (((CAClef*)_musElementList[i])->centerPitch());
	}

	return -1;
}

/*!
	Returns the last playable element (eg. note or rest) in the voice.
	
	\sa lastNotePitch()
*/
CAPlayable* CAVoice::lastPlayableElt() {
	for (int i=_musElementList.size()-1; i>=0; i--) {
		if (_musElementList[i]->isPlayable())
			return static_cast<CAPlayable*>(_musElementList[i]);
	}
	
	return 0;
}

/*!
	Returns the note in the voice.
	
	\sa lastNotePitch()
*/
CANote* CAVoice::lastNote() {
	for (int i=_musElementList.size()-1; i>=0; i--) {
		if (_musElementList[i]->musElementType()==CAMusElement::Note)
			return static_cast<CANote*>(_musElementList[i]);
	}
	
	return 0;
}

/*!
	Returns true, if this voice contains a note with the given \a pitch at the given
	\a startTime.
	
	This is useful when inserting a note and there needs to be determined if a user is
	adding a note to a chord and the note is maybe already there. Note's accidentals
	are ignored.
*/
bool CAVoice::containsPitch(int pitch, int startTime) {
	 for (int i=0; i<_musElementList.size(); i++)
	 	if ( (_musElementList[i]->musElementType() == CAMusElement::Note) && (_musElementList[i]->timeStart() == startTime) && (((CANote*)_musElementList[i])->pitch() == pitch) )
			return true;
		
	return false;
}

/*!
	Returns a list of pointers to actual music elements which have the given \a
	startTime and are of given \a type.
	This is useful for querying for eg. If a new key signature exists at the certain
	point in time.
*/
QList<CAMusElement*> CAVoice::getEltByType(CAMusElement::CAMusElementType type, int startTime) {
	QList<CAMusElement*> eltList;
	
	int i;
	for (i=0; i < _musElementList.size() && _musElementList[i]->timeStart() < startTime; i++);	// seek to the start of the music elements with the given time
	
	while (i<_musElementList.size() && _musElementList[i]->timeStart()==startTime) {	// create a list of music elements with the given time
		if (_musElementList[i]->musElementType() == type)
			eltList << _musElementList[i];
		i++;
	}
	
	return eltList;
}

/*!
	Returns pointer to the music element before the given \a elt or 0, if the previous
	music element doesn't exist.
*/
CAMusElement *CAVoice::eltBefore(CAMusElement *elt) {
	int idx = _musElementList.indexOf(elt);
	
	if (--idx<0) //if the element wasn't found or was the first element
		return 0;
	
	return _musElementList[idx];
}

/*!
	Returns pointer to the music element after the given \a elt or 0, if the next music
	element doesn't exist.
*/
CAMusElement *CAVoice::eltAfter(CAMusElement *elt) {
	int idx = _musElementList.indexOf(elt);
	
	if (idx==-1) //the element wasn't found
		return 0;
	
	if (++idx==_musElementList.size())	//last element in the list
		return 0;
	
	return _musElementList[idx];
}

/*!
	Returns a list of notes and rests (chord) in the given voice in the given
	time slice \a time.
	
	This is useful for determination of the harmony at certain point in time.
	
	\sa CAStaff:getChord(), CASheet::getChord()
*/
QList<CAPlayable*> CAVoice::getChord(int time) {
	int i;
	for (i=0; i<_musElementList.size() && (_musElementList[i]->timeEnd()<=time || !_musElementList[i]->isPlayable()); i++);
	if (i!=_musElementList.size()) {
		if (_musElementList[i]->musElementType()==CAMusElement::Note) {	// music element is a note
			//! \todo Casting QList<CANote*> to QList<CAPlayable*> doesn't work?! :( Do the conversation manually. This is slow. -Matevz
			QList<CANote*> list = ((CANote*)_musElementList[i])->chord();
			QList<CAPlayable*> ret;
			for (int i=0; i<list.size(); i++)
				ret << list[i];
			return ret;
		}
		else {	// music element is a rest
			QList<CAPlayable*> ret;
			ret << (CARest*)_musElementList[i];
			return ret;
		}
	}
	
	return QList<CAPlayable*>();
}

/*!
	Adds a music element \a elt to the end of the music elements list.
	
	Returns true, if the element was successfully added; false otherwise.
*/
bool CAVoice::appendMusElement(CAMusElement *elt) {
	return insertMusElementBefore(elt, 0, false, false);
}

/*!
	Inserts a music element \a elt at the beginning of the music elements list.
	
	Returns true, if the element was successfully added; false otherwise.
*/
bool CAVoice::prependMusElement(CAMusElement *elt) {
	return insertMusElementAfter(elt, 0, true, false);
}

/*!
	Returns a list of all notes in the voice.
*/
QList<CANote*> CAVoice::noteList() {
	QList<CANote*> list;
	for (int i=0; i<_musElementList.size(); i++)
		if (_musElementList[i]->musElementType()==CAMusElement::Note)
			list << ((CANote*)_musElementList[i]);
	
	return list;
}

/*!
	Returns a pointer to the next note with the higher timeStart than the given one.
	Returns 0, if the such a note doesn't exist.
*/
CANote *CAVoice::findNextNote( int timeStart ) {
	CANote *n = 0;
	int i;
	for (i=0;
	     i<_musElementList.size() &&
	     	(_musElementList[i]->musElementType()!=CAMusElement::Note ||
	     	 _musElementList[i]->timeStart()<=timeStart
	     	);
	     i++);
	
	if (i<_musElementList.size())
		return static_cast<CANote*>(_musElementList[i]);
	else
		return 0;
}

/*!
	Returns a pointer to the previous note with the lower timeStart than the given one.
	Returns 0, if the such a note doesn't exist.
*/
CANote *CAVoice::findPrevNote( int timeStart ) {
	CANote *n = 0;
	int i;
	for (i=_musElementList.size()-1;
	     i>-1 &&
	     	(_musElementList[i]->musElementType()!=CAMusElement::Note ||
	     	 _musElementList[i]->timeStart()>=timeStart
	     	);
	     i--);
	
	if (i>-1)
		return static_cast<CANote*>(_musElementList[i]);
	else
		return 0;
}

/*!
	Sets the stem direction and update slur directions in all the notes in the voice.
*/
void CAVoice::setStemDirection(CANote::CAStemDirection direction) {
	_stemDirection = direction;
}

/*!
	\var CAVoice::_voiceNumber
	Preferred direction of stems for the notes inside the voice. This should be Neutral, if the voice is alone, Up, if the voice is the first voice, Down, if not. Preferred is not used here.

	\sa CANote::CAStemDirection
*/

/*!
	\fn CAVoice::musElementList()
	Returns the list of music elements in the voice.
	
	\sa _musElementList
*/

/*!
	\var CAVoice::_staff
	Staff which this voice belongs to.
	
	\sa staff()
*/

/*!
	\var CAVoice::_voiceNumber
	Voice number in the staff starting at 1.
	
	\sa voiceNumber()
*/
