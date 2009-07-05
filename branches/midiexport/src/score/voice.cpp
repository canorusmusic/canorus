/*!
	Copyright (c) 2006-2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/muselement.h"
#include "score/voice.h"
#include "score/staff.h"
#include "score/keysignature.h"
#include "score/timesignature.h"
#include "score/clef.h"
#include "score/note.h"
#include "score/rest.h"
#include "score/playable.h"
#include "score/lyricscontext.h"
#include "score/slur.h"
#include "score/mark.h"
#include "score/tempo.h"
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
		_voiceNumber = staff->voiceList().size()+1;
	} else {
		_voiceNumber = voiceNumber;
	}
	_stemDirection = stemDirection;

	_midiChannel = ((staff && staff->sheet()) ? CAMidiDevice::freeMidiChannel( staff->sheet() ) : 0);
	_midiProgram = 0;
}

/*!
	Clears and destroys the current voice.
	This also destroys all non-shared music elements held by the voice.

	\sa clear()
*/
CAVoice::~CAVoice() {
	clear();

	QList<CALyricsContext*> lc = lyricsContextList();
	for (int i=0; i<lc.size(); i++) {
		lc[i]->setAssociatedVoice( 0 );
	}

	if (staff()) {
		staff()->removeVoice(this);
	}
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
	setStemDirection( voice->stemDirection() );
	setMidiChannel( voice->midiChannel() );
	setMidiProgram( voice->midiProgram() );
	setLyricsContexts( voice->lyricsContextList() );
}

/*!
	Destroys all non-shared music elements held by the voice.

	When clearing the whole staff, make sure the voice is *deleted*.
	It is automatically removed from the staff - in voice's destructor.
*/
void CAVoice::clear() {
	while ( _musElementList.size() ) {
		// deletes an element only if it's not present in other voices or we're deleting the last voice
		if ( _musElementList.front()->isPlayable() || staff() && staff()->voiceList().size()<2 )
			delete _musElementList.front(); // CAMusElement's destructor removes it from the list
		else
			_musElementList.removeFirst();
	}
}

/*!
	Appends a music element \a elt at the end of the voice.

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

	if ( eltAfter && eltAfter->musElementType()==CAMusElement::Note &&
	     static_cast<CANote*>(eltAfter)->getChord().size() ) // if eltAfter is note, it should always be the FIRST note in the chord
		eltAfter = static_cast<CANote*>(eltAfter)->getChord().front();

	bool res;
	if ( !elt->isPlayable() ) {

		// insert a sign

		elt->setTimeStart( eltAfter?eltAfter->timeStart():lastTimeEnd() );
		res = insertMusElement( eltAfter, elt );

		// calculate note positions in staff when inserting a new clef
		if ( elt->musElementType()==CAMusElement::Clef ) {
			for ( int i=musElementList().indexOf(elt)+1; i < musElementList().size(); i++ ) {
				if ( musElementList()[i]->musElementType()==CAMusElement::Note )
					static_cast<CANote*>(musElementList()[i])->setDiatonicPitch( static_cast<CANote*>(musElementList()[i])->diatonicPitch() );
			}
		}

	} else if ( elt->musElementType()==CAMusElement::Note && eltAfter && eltAfter->musElementType()==CAMusElement::Note && addToChord ) {

		// add a note to chord

		res = addNoteToChord( static_cast<CANote*>(elt), static_cast<CANote*>(eltAfter) );

	} else {

		// insert a note somewhere in between, append or prepend

		elt->setTimeStart( eltAfter?(eltAfter->timeStart()):lastTimeEnd() );
		res = insertMusElement( eltAfter, elt );
		updateTimes( musElementList().indexOf(elt)+1, elt->timeLength(), true );

	}

 	return res;
}

/*!
	Inserts a note/rest in a tuplet/voice. If the result should not be a chord the element
	found will be deleted and replaced. This function probably should also work for non
	tuplets.

	Currently only adding notes, and with the basic tuplet timelength are implemented.
*/
CAPlayable* CAVoice::insertInTupletAndVoiceAt( CAPlayable *reference, CAPlayable *p ) {
	int t = reference->timeStart();
	int rtype = static_cast<CAMusElement*>(reference)->musElementType();
	int ptype = static_cast<CAMusElement*>(p)->musElementType();
	CATuplet* tup = reference->tuplet();

	CAVoice* voice = reference->voice();
	CAMusElement* next = voice->next(static_cast<CAMusElement*>(reference));
	p->setTimeStart( t );

	if (rtype == CAMusElement::Rest) {

		voice->insert( next, static_cast<CAMusElement*>(p), false );


		if ( tup ) { // remove the rest from the tuplet and add the note
			tup->removeNote(reference);
			reference->setTuplet(0);
			tup->addNote(p);

			reference->voice()->remove( reference, true );
			tup->assignTimes();
		}

	} else {
		// add the note to a chord
		voice->insert( reference, static_cast<CAMusElement*>(p), true );

		if ( tup ) {
			tup->addNote(p);
			tup->assignTimes();
		}
	}
	return p;
}


/*!
	Returns a pointer to the clef which the given \a elt belongs to.
	Returns 0, if no clefs placed yet.
*/
CAClef* CAVoice::getClef(CAMusElement *elt) {
	if (!elt || !musElementList().contains(elt))
		elt = lastMusElement();

	while ( elt && (elt->musElementType() != CAMusElement::Clef) && (elt = previous(elt)) );

	return static_cast<CAClef*>(elt);
}

/*!
	Returns a pointer to the time signature which the given \a elt belongs to.
	Returns 0, if no time signatures placed yet.
*/
CATimeSignature* CAVoice::getTimeSig(CAMusElement *elt) {
	if (!elt || !musElementList().contains(elt))
		elt = lastMusElement();

	while ( elt && (elt->musElementType() != CAMusElement::TimeSignature) && (elt = previous(elt)) );

	return static_cast<CATimeSignature*>(elt);
}

/*!
	Returns a pointer to the key signature which the given \a elt belongs to.
	Returns 0, if no key signatures placed yet.
*/
CAKeySignature* CAVoice::getKeySig(CAMusElement *elt) {
	if (!elt || !musElementList().contains(elt))
		elt = lastMusElement();

	while ( elt && (elt->musElementType() != CAMusElement::KeySignature) && (elt = previous(elt)) );

	return static_cast<CAKeySignature*>(elt);
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
			for (int i=0; i<staff()->voiceList().size(); i++) {
				staff()->voiceList()[i]->_musElementList.removeAll(elt);
			}
			// remove it from the references list
			if (elt->musElementType() == CAMusElement::KeySignature )  staff()->removeKeySignatureReference( elt ); else
			if (elt->musElementType() == CAMusElement::TimeSignature ) staff()->removeTimeSignatureReference( elt ); else
			if (elt->musElementType() == CAMusElement::Clef )          staff()->removeClefReference( elt );
		} else {
			// element is playable
			if ( elt->musElementType()==CAMusElement::Note ) {
				CANote *n = static_cast<CANote*>(elt);
				if ( n->isPartOfChord() && n->isFirstInChord() ) {
					// if the note is the first in the chord, the slurs and marks should be relinked to the 2nd in the chord
					CANote *prevNote = n->getChord().at(1);
					prevNote->setSlurStart( n->slurStart() );
					prevNote->setSlurEnd( n->slurEnd() );
					prevNote->setPhrasingSlurStart( n->phrasingSlurStart() );
					prevNote->setPhrasingSlurEnd( n->phrasingSlurEnd() );

					for (int i=0; i<n->markList().size(); i++) {
						if ( n->markList()[i]->isCommon() ) {
							prevNote->addMark( n->markList()[i] );
							n->markList()[i]->setAssociatedElement( prevNote );
							n->removeMark( n->markList()[i--] );
						}
					}
				} else if ( !(n->isPartOfChord()) ) {
					if ( n->slurStart() ) delete n->slurStart();
					if ( n->slurEnd() ) delete n->slurEnd();
					if ( n->phrasingSlurStart() ) delete n->phrasingSlurStart();
					if ( n->phrasingSlurEnd() ) delete n->phrasingSlurEnd();
					if ( n->tuplet() ) delete n->tuplet();

					updateTimes( musElementList().indexOf(elt)+1, elt->timeLength()*(-1), updateSigns ); // shift back timeStarts of playable elements after it
				}
			} else {
				if ( elt->isPlayable() && static_cast<CAPlayable*>(elt)->tuplet() ) delete static_cast<CAPlayable*>(elt)->tuplet();
				updateTimes( musElementList().indexOf(elt)+1, elt->timeLength()*(-1), updateSigns ); // shift back timeStarts of playable elements after it
			}

			_musElementList.removeAll(elt);          // removes the element from the voice music element list
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
		_musElementList.push_back(elt);
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
	_musElementList.insert(i, elt);

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
	for ( i=0; i<chord.size() && chord[i]->diatonicPitch().noteName() < note->diatonicPitch().noteName(); i++ );

	_musElementList.insert( idx+i, note );
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
	, or ' octave marks need to be determined.

	\sa lastPlayableElt()
*/
CADiatonicPitch CAVoice::lastNotePitch(bool inChord) {
	for (int i=_musElementList.size()-1; i>=0; i--) {
		if (_musElementList[i]->musElementType()==CAMusElement::Note) {
			if (!((CANote*)_musElementList[i])->isPartOfChord() || !inChord)	// the note is not part of the chord
				return (static_cast<CANote*>(_musElementList[i])->diatonicPitch() );
			else {
				int chordTimeStart = _musElementList[i]->timeStart();
				int j;
				for (j=i;
				     (j>=0 && _musElementList[j]->musElementType()==CAMusElement::Note && _musElementList[j]->timeStart()==chordTimeStart);
				     j--);

				return (static_cast<CANote*>(_musElementList[j+1])->diatonicPitch() );
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


//! \A common binary search Algorithm with its pseudocode

bool CAVoice::binarySearch_startTime(int time, int& position) {

	int low = 0, high = _musElementList.size()-1, midpoint = 0;
	while (low <= high) {
		midpoint = (low + high) / 2;
		if (time == _musElementList[midpoint]->timeStart()) {
			position = midpoint;
			return true;
		}
		else if (time < _musElementList[midpoint]->timeStart())
			high = midpoint - 1;
		else
			low = midpoint + 1;
	}
	return false;
}


/*!
	Returns a music element which has the given \a startTime and \a type.
	This is useful for querying for eg. If a barline exists at the certain
	point in time.
*/
CAMusElement *CAVoice::getOneEltByType(CAMusElement::CAMusElementType type, int startTime) {

	int i;
	for (i=0; i < _musElementList.size() && _musElementList[i]->timeStart() < startTime; i++);	// seek to the start of the music elements with the given time

	while (i<_musElementList.size() && _musElementList[i]->timeStart()==startTime) {	// create a list of music elements with the given time
		if (_musElementList[i]->musElementType() == type)
			return _musElementList[i];
		i++;
	}

	return 0;
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
	Returns a music elements which is at or left (not past)
	the given \a startTime and of given \a type.
	This is useful for querying for eg. which is the barline before a certain
	point in time.

*/
CAMusElement *CAVoice::getOnePreviousByType(CAMusElement::CAMusElementType type, int startTime) {

	int i;
	for (i= _musElementList.size()-1;
			i >= 0 && _musElementList[i]->timeStart() > startTime; i--);	// seek to the most right of the music elements with the given time
	while (i >=0 && _musElementList[i]->timeStart() <= startTime) {	// create a list of music elements not past the given time
		if (_musElementList[i]->musElementType() == type)
			return _musElementList[i];
		i--;
	}
	return 0;
}

/*!
	Returns a list of pointers to actual music elements which are at or left (not past)
	the given \a startTime and are of given \a type.
	This is useful for querying for eg. which key pitch is in effect before a certain
	point in time.

	A list from time 0 until startTime is created which is
	questionable regarding need and efficiency.
*/
QList<CAMusElement*> CAVoice::getPreviousByType(CAMusElement::CAMusElementType type, int startTime) {
	QList<CAMusElement*> eltList;

	int i;
	for (i= _musElementList.size()-1;
			i >= 0 && _musElementList[i]->timeStart() > startTime; i--);	// seek to the most right of the music elements with the given time
	while (i >=0 && _musElementList[i]->timeStart() <= startTime) {	// create a list of music elements not past the given time
		if (_musElementList[i]->musElementType() == type)
			eltList.prepend(_musElementList[i]);
		i--;
	}


	return eltList;
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
	Returns a list of music elements inside the given bar.
	The return list consists of all music elements between two barlines excluding the first barline.
	Expression marks and other non-standalone elements are excluded as well.
	The parameter \a time is any time of music elements inside the bar.

	This function is usually called when double clicking on the score.
 */
QList<CAMusElement*> CAVoice::getBar( int time ) {
	QList<CAPlayable*> chord = getChord(time);
	QList<CAMusElement*> ret;

	if ( !chord.size() ) {
		return ret;
	}

	// search left
	CAMusElement *curElt = previous( chord[0] );
	while ( curElt && curElt->musElementType()!=CAMusElement::Barline ) {
		ret.append( curElt );
		curElt = previous(curElt);
	}

	ret.append( chord[0] );

	curElt = next( chord[0] );
	while ( curElt && curElt->musElementType()!=CAMusElement::Barline ) {
		ret.append( curElt );
		curElt = next(curElt);
	}

	if (curElt) { // last elt is barline
		ret.append( curElt );
	}

	return ret;
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
	Returns pointer to the music element after the given \a elt or 0, if the next music
	element doesn't exist.

	If \elt is null, it returns the first element in the voice.
*/
CAMusElement *CAVoice::next(CAMusElement *elt) {
	if(musElementList().isEmpty())
		return 0;
	if (elt) {
		int idx = _musElementList.indexOf(elt);

		if (idx==-1) //the element wasn't found
			return 0;

		if (++idx==_musElementList.size())	//last element in the list
			return 0;

		return _musElementList[idx];
	} else {
		return _musElementList.first();
	}
}

/*!
	Returns the first element of type \a type after the given \a elt or Null if
	such an element doesn't exist.

	If \a elt is Null, it returns the first element with such a type in the voice.

	\sa previousByType()
 */
CAMusElement *CAVoice::nextByType( CAMusElement::CAMusElementType type, CAMusElement *elt ) {
	while ( (elt = next(elt)) && (elt->musElementType() != type) );

	return elt;
}

/*!
	Returns the first element of type \a type before the given \a elt or Null if
	such an element doesn't exist.

	If \a elt is Null, it returns the last element with such a type in the voice.

	\sa previousByType()
 */
CAMusElement *CAVoice::previousByType( CAMusElement::CAMusElementType type, CAMusElement *elt ) {
	while ( (elt = previous(elt)) && (elt->musElementType() != type) );

	return elt;
}

/*!
	Returns pointer to the music element before the given \a elt or 0, if the previous
	music element doesn't exist.

	If \elt is null, it returns the last element in the voice.
*/
CAMusElement *CAVoice::previous(CAMusElement *elt) {
	if(musElementList().isEmpty())
		return 0;
	if (elt) {
		int idx = _musElementList.indexOf(elt);

		if (--idx<0) //if the element wasn't found or was the first element
			return 0;

		return _musElementList[idx];
	} else {
		return _musElementList.last();
	}
}

/*!
	Returns a pointer to the next note with the strictly higher timeStart than the given one.
	Returns 0, if the such a note doesn't exist.
*/
CANote *CAVoice::nextNote( int timeStart ) {
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
	Returns a pointer to the previous note with the strictly lower timeStart than the given one.
	Returns 0, if the such a note doesn't exist.
*/
CANote *CAVoice::previousNote( int timeStart ) {
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
	Returns a pointer to the next rest with the strictly higher timeStart than the given one.
	Returns 0, if the such a note doesn't exist.
*/
CARest *CAVoice::nextRest(int timeStart) {
	int i;
	for (i=0;
	     i<_musElementList.size() &&
	     	(_musElementList[i]->musElementType()!=CAMusElement::Rest ||
	     	 _musElementList[i]->timeStart()<=timeStart
	     	);
	     i++);

	if (i<_musElementList.size())
		return static_cast<CARest*>(_musElementList[i]);
	else
		return 0;
}

/*!
	Returns a pointer to the previous rest with the strictly lower timeStart than the given one.
	Returns 0, if the such a note doesn't exist.
*/
CARest *CAVoice::previousRest(int timeStart) {
	int i;
	for (i=_musElementList.size()-1;
	     i>-1 &&
	     	(_musElementList[i]->musElementType()!=CAMusElement::Rest ||
	     	 _musElementList[i]->timeStart()>=timeStart
	     	);
	     i--);

	if (i>-1)
		return static_cast<CARest*>(_musElementList[i]);
	else
		return 0;
}

/*!
	Returns a pointer to the next playable element with the strictly higher timeStart than the given one.
	Returns 0, if the such a note doesn't exist.
*/
CAPlayable *CAVoice::nextPlayable(int timeStart) {
	int i;
	for (i=0;
	     i<_musElementList.size() &&
	     	(!_musElementList[i]->isPlayable() ||
	     	 _musElementList[i]->timeStart()<=timeStart
	     	);
	     i++);

	if (i<_musElementList.size())
		return static_cast<CAPlayable*>(_musElementList[i]);
	else
		return 0;
}

/*!
	Returns a pointer to the previous playable with the strictly lower timeStart than the given one.
	Returns 0, if the such a note doesn't exist.
*/
CAPlayable *CAVoice::previousPlayable(int timeStart) {
	int i;
	for (i=_musElementList.size()-1;
	     i>-1 &&
	     	(!_musElementList[i]->isPlayable() ||
	     	 _musElementList[i]->timeStart()>=timeStart
	     	);
	     i--);

	if (i>-1)
		return static_cast<CAPlayable*>(_musElementList[i]);
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
		if ( signsToo || musElementList()[i]->isPlayable() ) {
			musElementList()[i]->setTimeStart( musElementList()[i]->timeStart() + length );
			for (int j=0; j<musElementList()[i]->markList().size(); j++) {
				CAMark *m = musElementList()[i]->markList()[j];
				if ( !m->isCommon() || musElementList()[i]->musElementType()!=CAMusElement::Note ||
				     static_cast<CANote*>(musElementList()[i])->isFirstInChord() )
					m->setTimeStart( musElementList()[i]->timeStart() + length );
			}
		}
}

/*!
	Fixes any inconsistencies between music elements:
	1) If a common (shared) mark is present only in non-first note of the chord, it's moved and assigned
	   to first note in the chord.
	   The exception are non-common marks (eg. fingering), which are assigned to each note separately.

	Returns True, if fixes were made or False otherwise.
*/
bool CAVoice::synchronizeMusElements() {
	for (int i=0; i<musElementList().size(); i++) {
		if ( musElementList()[i]->musElementType()==CAMusElement::Note &&
		     musElementList()[i]->markList().size() &&
		     static_cast<CANote*>(musElementList()[i])->isPartOfChord() ) {
			QList<CAMark*> marks; // list of shared marks
			QList<CANote*> chord = static_cast<CANote*>(musElementList()[i])->getChord();

			// gather a list of marks and remove them from the chord
			for ( int j=0; j<chord.size(); j++ ) {
				for ( int k=0; k<chord[j]->markList().size(); k++ ) {
					if ( chord[j]->markList()[k]->isCommon() ) {
						chord[j]->markList()[k]->setAssociatedElement( chord.first() );

						if ( !marks.contains(chord[j]->markList()[k]) )
							marks << chord[j]->markList()[k];

						chord[j]->removeMark( chord[j]->markList()[k] );
					}
				}
			}

			// add marks back to the chord
			for (int k=0; k<marks.size(); k++) {
				chord.first()->addMark(marks[k]);
			}

			// move at the end of the chord
			i += (chord.size() - chord.indexOf( static_cast<CANote*>(musElementList()[i]) ));
		}
	}
}

/*!
	Returns true, if this voice contains a note with the given \a pitch notename at the
	given \a timeStart.

	This is useful when inserting a note and there needs to be determined if a user is
	adding a note to a chord and the note is maybe already there. Note's accidentals
	are ignored.
*/
bool CAVoice::containsPitch( int noteName, int timeStart ) {
	for (int i=0 ;i<_musElementList.size(); i++) {
		if ( _musElementList[i]->timeStart()==timeStart &&
		     _musElementList[i]->musElementType()==CAMusElement::Note &&
		     static_cast<CANote*>(_musElementList[i])->diatonicPitch().noteName()==noteName )
			return true;
	}

	return false;
}

/*!
	Returns true, if this voice contains a note with the given diatonic \a pitch at the
	given \a timeStart.

	This is useful when inserting a note and there needs to be determined if a user is
	adding a note to a chord and the note is maybe already there.
*/
bool CAVoice::containsPitch( CADiatonicPitch p, int timeStart ) {
	for (int i=0 ;i<_musElementList.size(); i++) {
		if ( _musElementList[i]->timeStart()==timeStart &&
		     _musElementList[i]->musElementType()==CAMusElement::Note &&
		     static_cast<CANote*>(_musElementList[i])->diatonicPitch()==p )
			return true;
	}
	return false;
}

/*!
	Returns the Tempo element active at the given time.
 */
CATempo *CAVoice::getTempo( int time ) {
	QList<CAPlayable*> chord = getChord(time);
	int curElt = -1;

	if ( chord.isEmpty() ) {
		curElt = musElementList().size()-1;
	} else {
		curElt = musElementList().indexOf(chord.last());
	}

	CATempo *tempo = 0;
	while (!tempo && curElt>=0) {
		for (int i=0; i<musElementList()[curElt]->markList().size(); i++) {
			if (musElementList()[curElt]->markList()[i]->markType()==CAMark::Tempo) {
				tempo = static_cast<CATempo*>(musElementList()[curElt]->markList()[i]);
			}
		}
		curElt--;
	}

	return tempo;
}

/*!
	Returns a list of pointers to key signatures which have the given \a startTime.
	This is useful for querying for eg. If a new key signature exists at the certain
	point in time.
*/
QList<CAMusElement*> CAVoice::getKeySignature(int startTime) {

	QList<CAMusElement*> eltList;
	int i;
	// seek to the start of the music elements with the given time
	for (i=0; i < staff()->keySignatureReferences().size() && staff()->keySignatureReferences()[i]->timeStart() < startTime; i++);

	// create a list of music elements with the given time
	while (i<staff()->keySignatureReferences().size() && staff()->keySignatureReferences()[i]->timeStart()==startTime) {
		eltList << staff()->keySignatureReferences()[i];
		i++;
	}

	return eltList;
}

/*!
	Returns a list of pointers to key signatures which have the given \a startTime.
	This is useful for querying for eg. If a new key signature exists at the certain
	point in time.
*/
QList<CAMusElement*> CAVoice::getTimeSignature(int startTime) {

	QList<CAMusElement*> eltList;
	int i;
	// seek to the start of the music elements with the given time
	for (i=0; i < staff()->timeSignatureReferences().size() && staff()->timeSignatureReferences()[i]->timeStart() < startTime; i++);

	// create a list of music elements with the given time
	while (i<staff()->timeSignatureReferences().size() && staff()->timeSignatureReferences()[i]->timeStart()==startTime) {
		eltList << staff()->timeSignatureReferences()[i];
		i++;
	}

	return eltList;
}

/*!
	Returns a list of pointers to key signatures which have the given \a startTime.
	This is useful for querying for eg. If a new key signature exists at the certain
	point in time.
*/
QList<CAMusElement*> CAVoice::getClef(int startTime) {

	QList<CAMusElement*> eltList;
	int i;
	// seek to the start of the music elements with the given time
	for (i=0; i < staff()->clefReferences().size() && staff()->clefReferences()[i]->timeStart() < startTime; i++);

	// create a list of music elements with the given time
	while (i<staff()->clefReferences().size() && staff()->clefReferences()[i]->timeStart()==startTime) {
		eltList << staff()->clefReferences()[i];
		i++;
	}

	return eltList;
}

/*!
	Returns a list of pointers to key signatures which are at or left (not past)
	the given \a startTime.
	This is useful for querying for eg. which key signature is in effect before a certain
	point in time.
*/
QList<CAMusElement*> CAVoice::getPreviousKeySignature(int startTime) {

	QList<CAMusElement*> eltList;
	int i;
	// seek to the most right of the music elements with the given time
	for (i= staff()->keySignatureReferences().size()-1;
			i >= 0 && staff()->keySignatureReferences()[i]->timeStart() > startTime; i--);
	// create a list of music elements not past the given time
	while (i >=0 && staff()->keySignatureReferences()[i]->timeStart() <= startTime) {
		eltList.prepend(staff()->keySignatureReferences()[i]);
		i--;
	}
	return eltList;
}

/*!
	Returns a list of pointers to key signatures which are at or left (not past)
	the given \a startTime.
	This is useful for querying for eg. which key signature is in effect before a certain
	point in time.
*/
QList<CAMusElement*> CAVoice::getPreviousTimeSignature(int startTime) {

	QList<CAMusElement*> eltList;
	int i;
	// seek to the most right of the music elements with the given time
	for (i= staff()->timeSignatureReferences().size()-1;
			i >= 0 && staff()->timeSignatureReferences()[i]->timeStart() > startTime; i--);
	// create a list of music elements not past the given time
	while (i >=0 && staff()->timeSignatureReferences()[i]->timeStart() <= startTime) {
		eltList.prepend(staff()->timeSignatureReferences()[i]);
		i--;
	}
	return eltList;
}

/*!
	Returns a list of pointers to clefs which are at or left (not past)
	the given \a startTime.
	This is useful for querying for eg. which clef is in effect before a certain
	point in time.
*/
QList<CAMusElement*> CAVoice::getPreviousClef(int startTime) {

	QList<CAMusElement*> eltList;
	int i;
	// seek to the most right of the music elements with the given time
	for (i= staff()->clefReferences().size()-1;
			i >= 0 && staff()->clefReferences()[i]->timeStart() > startTime; i--);
	// create a list of music elements not past the given time
	while (i >=0 && staff()->clefReferences()[i]->timeStart() <= startTime) {
		eltList.prepend(staff()->clefReferences()[i]);
		i--;
	}
	return eltList;
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
	\fn CAVoice::voiceNumber()
	Voice number in the staff starting at 1.

	Voice number is 1, if no staff defined.
*/
