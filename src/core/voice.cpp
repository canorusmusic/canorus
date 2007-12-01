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
	It is automatically removed from the staff - in voice's destructor.
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
	Appends a playable \a elt (note or rest) at the end of the voice.
	
	If \a addToChord is True and the appended element is note, the note is added to the chord
	instead of added after the chord. If appended element is rest, \a addToChord is ignored.
	
	Appended element's timeStart is changed respectively.
	
	\note Due to speed issues, voices are NOT synchronized for every inserted element. User
	should manually call CAStaff::synchronizeVoices().
	
	\sa insert()
*/
void CAVoice::append( CAMusElement *elt, bool addToChord ) {
	CAMusElement *last = (musElementList().size()?musElementList().last():0);
	
	if ( elt->musElementType()==CAMusElement::Note && last &&
	     last->musElementType()==CAMusElement::Note &&
	     addToChord ) {
		elt->setTimeStart( last->timeStart() );
		addNoteToChord( static_cast<CANote*>(elt), static_cast<CANote*>(last) );
	} else {
		elt->setTimeStart( last?last->timeEnd():0 );
		insertMusElement( 0, elt );
	}
}

/*!
	Adds the given element \a elt to the voice before the given \a eltAfter. If \a eltAfter is null,
	the element is appended.
	
	If \a elt is non-playable, it eventually does the same as insertMusElement().
	
	If \a elt is a note and addToChord is True, the eltAfter should also be a note and the \a elt is
	then added to the chord which eltAfter is part of.
	
	If \a elt is other playable element, it is appropriately added before the \a eltAfter. \a addToChord is ignored.
	
	Inserted element's timeStart is correctly changed.
	
	Returns True, if the insertion action was successfully made, otherwise False.
	
	\note Due to speed issues, voices are NOT synchronized for every inserted element. User
	should manually call CAStaff::synchronizeVoices().
	
	\sa insertMusElement()
*/
bool CAVoice::insert( CAMusElement *eltAfter, CAMusElement *elt, bool addToChord ) {
	if ( !elt )
		return false;
	
	if ( eltAfter && eltAfter->musElementType()==CAMusElement::Note ) // if eltAfter is note, it should always be the FIRST note in the chord
		eltAfter = static_cast<CANote*>(eltAfter)->getChord().front();
	
	bool res;
	if ( !elt->isPlayable() ) {
		
		// insert a sign
		
		elt->setTimeStart( eltAfter?eltAfter->timeStart():lastTimeEnd() );
		res = insertMusElement( eltAfter, elt );
		
		// calculate note positions in staff when inserting a new clef
		if ( elt->musElementType()==CAMusElement::Clef ) {
			for ( int i=musElementList().indexOf(elt)+1; i < musElementCount(); i++ ) {
				if ( musElementList()[i]->musElementType()==CAMusElement::Note )
					static_cast<CANote*>(musElementList()[i])->setPitch( static_cast<CANote*>(musElementList()[i])->pitch() );
			}
		}
		
	} else if ( elt->musElementType()==CAMusElement::Note && eltAfter && eltAfter->musElementType()==CAMusElement::Note && addToChord ) {
		
		// add a note to chord
		
		res = addNoteToChord( static_cast<CANote*>(elt), static_cast<CANote*>(eltAfter) );
		
	} else {
		
		// insert a note somewhere in between, append or prepend
		
		elt->setTimeStart( eltAfter?eltAfter->timeStart():lastTimeEnd() );
		res = insertMusElement( eltAfter, elt );
		updateTimes( musElementList().indexOf(elt)+1, elt->timeLength(), true );
		
	}
	
 	return res;
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
	Removes the given music element \a elt from this voice, if the element is playable or from
	all the voices in the staff, if non-playable and part of the staff.
	
	If \a updateSigns is True, startTimes of elements after the removed one are decreased
	including the shared signs. Otherwise only timeStarts for playable elements are effected.
	
	\warning This function doesn't destroy the object, but only removes its
	reference in the voice.
	
	\note Due to speed issues, voices are NOT synchronized for every inserted element. User
	should manually call CAStaff::synchronizeVoices().
	
	Returns true, if the element was found and removed; otherwise false.
*/
bool CAVoice::remove( CAMusElement *elt, bool updateSigns ) {
	if ( _musElementList.contains(elt) ) {	// if the search element is found
		if ( !elt->isPlayable() && staff() ) {          // element is shared - remove it from all the voices
			for (int i=0; i<staff()->voiceCount(); i++) {
				staff()->voiceAt(i)->musElementList().removeAll(elt);
			}
		} else {
			if ( elt->musElementType()==CAMusElement::Note ) {
				CANote *n = static_cast<CANote*>(elt);
				if ( n->isPartOfTheChord() && n->isFirstInTheChord() ) {
					CANote *prevNote = n->getChord().at(1);
					prevNote->setSlurStart( n->slurStart() );
					prevNote->setSlurEnd( n->slurEnd() );
					prevNote->setPhrasingSlurStart( n->phrasingSlurStart() );
					prevNote->setPhrasingSlurEnd( n->phrasingSlurEnd() );
				} else if ( !(n->isPartOfTheChord()) ) {
					if ( n->slurStart() ) delete n->slurStart();
					if ( n->slurEnd() ) delete n->slurEnd();
					if ( n->phrasingSlurStart() ) delete n->phrasingSlurStart();
					if ( n->phrasingSlurEnd() ) delete n->phrasingSlurEnd();
				}
			}
			
			updateTimes( musElementList().indexOf(elt)+1, elt->timeLength()*(-1), updateSigns ); // shift back timeStarts of playable elements after it
			musElementList().removeAll(elt);          // removes the element from the voice music element list
		}
		
		return true;
	} else {
		return false;
	}
}

/*!
	Inserts the \a elt before the given \a eltAfter. If \a eltAfter is Null, it
	appends the element.
	
	Returns True, if \a eltAfter was found and the elt was inserted/appended; otherwise False.
*/
bool CAVoice::insertMusElement( CAMusElement *eltAfter, CAMusElement *elt ) {
	if (!eltAfter || !_musElementList.size()) {
		musElementList().push_back(elt);
		return true;
	}
	
	int i = musElementList().indexOf( eltAfter );
	
	// if element wasn't found and the element before is slur
	if ( eltAfter->musElementType()==CAMusElement::Slur && i==-1 )
		i = musElementList().indexOf( static_cast<CASlur*>(eltAfter)->noteEnd() );
	
	if (i==-1) {
		// eltBefore still wasn't found, return False
		return false;
	}
	
	// eltBefore found, insert it
	musElementList().insert(i, elt);
	
	return true;
}

/*!
	Adds a \a note to an already existing \a referenceNote chord or a single note and
	creates a chord out of it.
	Notes in a chord always need to be sorted by pitch rising.
	Chord in Canorus isn't its own structure but simply a list of notes sharing the
	same start time.
	
	The inserted \a note properteis timeStart, timeLength, dotted and playableLength
	change according to other notes in the chord.
	
	Returns True, if a referenceNote was found and a note was added; otherwise False.
	
	\sa CANote::chord()
*/
bool CAVoice::addNoteToChord(CANote *note, CANote *referenceNote) {
	int idx = _musElementList.indexOf(referenceNote);
	
	if (idx==-1)
		return false;
	
	QList<CANote*> chord = referenceNote->getChord();
	idx = _musElementList.indexOf(chord.first());
	
	int i;
	for ( i=0; i<chord.size() && chord[i]->pitch() < note->pitch(); i++ );
	
	_musElementList.insert( idx+i, note );
	note->setDotted( referenceNote->dotted() );
	note->setPlayableLength( referenceNote->playableLength() );
	note->setTimeLength( referenceNote->timeLength() );
	note->setTimeStart( referenceNote->timeStart() );
	note->setStemDirection( referenceNote->stemDirection() );
	
	return true;
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
CAMusElement *CAVoice::previous(CAMusElement *elt) {
	int idx = _musElementList.indexOf(elt);
	
	if (--idx<0) //if the element wasn't found or was the first element
		return 0;
	
	return _musElementList[idx];
}

/*!
	Returns pointer to the music element after the given \a elt or 0, if the next music
	element doesn't exist.
*/
CAMusElement *CAVoice::next(CAMusElement *elt) {
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
			QList<CANote*> list = static_cast<CANote*>(_musElementList[i])->getChord();
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
	Generates a list of all the notes and chords in the voice.
	
	This is useful for harmony analysis.
*/
QList<CANote*> CAVoice::getNoteList() {
	QList<CANote*> list;
	for (int i=0; i<_musElementList.size(); i++)
		if (_musElementList[i]->musElementType()==CAMusElement::Note)
			list << ((CANote*)_musElementList[i]);
	
	return list;
}

/*!
	Generates a list of all the notes and chords in the voice.
	
	This is useful when importing a specific voice and all the shared elements should be
	completely repositioned.
*/
QList<CAMusElement*> CAVoice::getSignList() {
	QList<CAMusElement*> list;
	for (int i=0; i<_musElementList.size(); i++)
		if ( !_musElementList[i]->isPlayable() )
			list << _musElementList[i];
	
	return list;
}

/*!
	Returns a pointer to the next note with the higher timeStart than the given one.
	Returns 0, if the such a note doesn't exist.
*/
CANote *CAVoice::nextNote( int timeStart ) {
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
CANote *CAVoice::previousNote( int timeStart ) {
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
	Updates times of playable elements and optionally \a signsToo after and including the given index
	\a idx for a delta \a length. The order of the elements stays intact.
	
	This method is usually called when inserting, removing or changing the music elements so they affect
	others.
*/
bool CAVoice::updateTimes( int idx, int length, bool signsToo ) {
	for (int i=idx; i<musElementList().size(); i++)
		if ( signsToo || musElementList()[i]->isPlayable() )
			musElementList()[i]->setTimeStart( musElementList()[i]->timeStart() + length );
}

/*!
	\fn void CAVoice::setStemDirection(CANote::CAStemDirection direction)
	
	Sets the stem direction and update slur directions in all the notes in the voice.
*/

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
