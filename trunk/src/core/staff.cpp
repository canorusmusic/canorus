/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QPainter>
#include <iostream>

#include "core/voice.h"
#include "core/staff.h"
#include "core/note.h"

/*!
	\class CAStaff
	\brief Represents a staff in the sheet
	
	This class represents usually an infinite long n-line staff where notes, rests, barlines, clefs and
	other music elements are placed.
	
	CAStaff is by hierarchy part of CASheet and can include various number of CAVoice objects.
	
	\sa CADrawableStaff, CASheet, CAVoice
*/

/*!
	Creates a new empty staff with parent sheet \a s, named \a name and \a numberOfLines.
	
	\warning By default, no voices are created where music elements can be put. Use addVoice() to
	append a new voice.
*/
CAStaff::CAStaff( const QString name, CASheet *s, int numberOfLines) : CAContext( name, s ) {
	_contextType = CAContext::Staff;
	_numberOfLines = numberOfLines;
	_name = name;
}

CAStaff::~CAStaff() {
	clear();
}

CAStaff *CAStaff::clone( CASheet *s ) {
	CAStaff *newStaff = new CAStaff( name(), s, numberOfLines() );
	
	// create empty voices
	for (int i=0; i<voiceCount(); i++) {
		newStaff->addVoice( voiceAt(i)->clone(newStaff) );
	}
	
	int eltIdx[voiceCount()]; for (int i=0; i<voiceCount(); i++) eltIdx[i]=0;
	CASlur *curTie[voiceCount()]; for (int i=0; i<voiceCount(); i++) curTie[i]=0;
	CASlur *curSlur[voiceCount()]; for (int i=0; i<voiceCount(); i++) curSlur[i]=0;
	CASlur *curPhrasingSlur[voiceCount()]; for (int i=0; i<voiceCount(); i++) curPhrasingSlur[i]=0;
	
	bool done=false;
	while (!done) {
		// append playable elements
		for (int i=0; i<voiceCount(); i++) {
			while ( eltIdx[i]<voiceAt(i)->musElementCount() && voiceAt(i)->musElementAt(eltIdx[i])->isPlayable() ) {
				CAPlayable *newElt = static_cast<CAPlayable*>(voiceAt(i)->musElementAt(eltIdx[i]))->clone( newStaff->voiceAt(i) );
				newStaff->voiceAt(i)->appendMusElement( newElt );
				
				// check tie
				if ( newElt->musElementType()==CAMusElement::Note &&
				     static_cast<CANote*>(voiceAt(i)->musElementAt(eltIdx[i]))->tieStart() ) {
					curTie[i] = static_cast<CANote*>(voiceAt(i)->musElementAt(eltIdx[i]))->tieStart()->clone();
					curTie[i]->setContext( newStaff );
					curTie[i]->setNoteStart( static_cast<CANote*>(newElt) );
					static_cast<CANote*>(newElt)->setTieStart(curTie[i]);
				}
				if ( newElt->musElementType()==CAMusElement::Note &&
				     static_cast<CANote*>(voiceAt(i)->musElementAt(eltIdx[i]))->tieEnd() ) {
					curTie[i]->setNoteEnd( static_cast<CANote*>(newElt) );
					static_cast<CANote*>(newElt)->setTieEnd(curTie[i]);
					curTie[i] = 0;
				}

				// check slur
				if ( newElt->musElementType()==CAMusElement::Note &&
				     static_cast<CANote*>(voiceAt(i)->musElementAt(eltIdx[i]))->slurStart() ) {
					curSlur[i] = static_cast<CANote*>(voiceAt(i)->musElementAt(eltIdx[i]))->slurStart()->clone();
					curSlur[i]->setContext( newStaff );
					curSlur[i]->setNoteStart( static_cast<CANote*>(newElt) );
					static_cast<CANote*>(newElt)->setSlurStart(curSlur[i]);
				}
				if ( newElt->musElementType()==CAMusElement::Note &&
				     static_cast<CANote*>(voiceAt(i)->musElementAt(eltIdx[i]))->slurEnd() ) {
					curSlur[i]->setNoteEnd( static_cast<CANote*>(newElt) );
					static_cast<CANote*>(newElt)->setSlurEnd(curSlur[i]);
					curSlur[i] = 0;
				}
				 
				// check phrasing slur
				if ( newElt->musElementType()==CAMusElement::Note &&
				     static_cast<CANote*>(voiceAt(i)->musElementAt(eltIdx[i]))->phrasingSlurStart() ) {
					curPhrasingSlur[i] = static_cast<CANote*>(voiceAt(i)->musElementAt(eltIdx[i]))->phrasingSlurStart()->clone();
					curPhrasingSlur[i]->setContext( newStaff );
					curPhrasingSlur[i]->setNoteStart( static_cast<CANote*>(newElt) );
					static_cast<CANote*>(newElt)->setPhrasingSlurStart(curPhrasingSlur[i]);					
				}
				if ( newElt->musElementType()==CAMusElement::Note &&
				     static_cast<CANote*>(voiceAt(i)->musElementAt(eltIdx[i]))->phrasingSlurEnd() ) {
					curPhrasingSlur[i]->setNoteEnd( static_cast<CANote*>(newElt) );
					static_cast<CANote*>(newElt)->setPhrasingSlurEnd(curPhrasingSlur[i]);					
					curPhrasingSlur[i] = 0;
				}
				
				eltIdx[i]++;
			}
		}
		
		// append non-playable elements (shared by all voices - only create clone of the first voice element and append it to all)
		if ( eltIdx[0]<voiceAt(0)->musElementCount() ) {
			CAMusElement *newElt = voiceAt(0)->musElementAt(eltIdx[0])->clone();
			newElt->setContext( newStaff );
			
			for (int i=0; i<voiceCount(); i++) {
				newStaff->voiceAt(i)->appendMusElement( newElt );
				eltIdx[i]++;
			}
		}
		
		// check if we're at the end
		done = true;
		for (int i=0; i<voiceCount(); i++) {
			if (eltIdx[i]<voiceAt(i)->musElementCount()) {
				done = false;
				break;
			}
		}
	}
	
	return newStaff;
}

/*!
	Returns the end of the last music element in the staff.
	
	\sa CAVoice::lastTimeEnd()
*/
int CAStaff::lastTimeEnd() {
	int maxTime = 0;
	for (int i=0; i<_voiceList.size(); i++)
		if (_voiceList[i]->lastTimeEnd() > maxTime)
			maxTime = _voiceList[i]->lastTimeEnd();
	
	return maxTime;
}

void CAStaff::clear() {
	while (_voiceList.size()) {
		delete _voiceList.front(); // CAVoice's destructor removes the voice from the list.
	}
}

/*!
	Adds a voice \a voice to the staff and sets its parent to this staff.
*/
void CAStaff::addVoice(CAVoice *voice) {
	int i;
	for ( i=0; i<_voiceList.size() && _voiceList[i]->voiceNumber() < voice->voiceNumber(); i++ );
	_voiceList.insert( i, voice );
	voice->setStaff(this);
}

/*!
	Inserts a sign \a sign to the staff at certain time.
	This method inserts the sign to all voices!
	This is the difference between inserting a sign or a playable note - note is present in a single
	voice and sign is shared - present in all the voice.
*/
void CAStaff::insertSign(CAMusElement *sign) {
	for (int i=0; i<_voiceList.size(); i++)
		_voiceList[i]->insertMusElement(sign);
}

/*!
	Inserts any sign \a sign (clef, siganutres, barline etc.) before the given \a eltAfter.
	This method inserts the sign to all voices!
	This is the difference between inserting a sign or a playable element - note is present in a
	single voice, sign in both.
	
	If \a force is true, finds the element with nearest time start and inserts it before.
	
	Returns true if \a sign was successfully added; otherwise false.
	
	\sa CAVoice::insertMusElementBefore(), insertSignAfter()
*/
bool CAStaff::insertSignBefore(CAMusElement *sign, CAMusElement *eltAfter, bool force) {
	bool error = false;
	for (int i=0; i<_voiceList.size(); i++) {
		if (!_voiceList[i]->insertMusElementBefore(sign, eltAfter, true, force))
			error = true;
	}
	
	return (!error);
}

/*!
	Inserts any sign \a sign (clef, siganutres, barline etc.) after the given \a eltBefore.
	This method inserts the sign to all voices!
	This is the difference between inserting a sign or a playable element - note is present in a
	single voice, sign in both.
	
	If \a force is true, finds the element with nearest time start and inserts it after.
	
	Returns true if \a sign was successfully added; otherwise false.
	
	\sa CAVoice::insertMusElementAfter(), insertSignBefore()
*/
bool CAStaff::insertSignAfter(CAMusElement *sign, CAMusElement *eltBefore, bool force) {
	bool error = false;
	for (int i=0; i<_voiceList.size(); i++) {
		if (!_voiceList[i]->insertMusElementAfter(sign, eltBefore, true, force))
			error = true;
	}
	
	return (!error);
}

/*!
	Removes the music element \a elt from all the voices.
	Also destroys an object, if \a cleanup is true (default).
	
	Returns true on successful removal; otherwise false.
*/
bool CAStaff::removeMusElement(CAMusElement *elt, bool cleanup) {
	bool success = false;
	for (int i=0; i<_voiceList.size(); i++)
		if (_voiceList[i]->removeElement(elt))
			success = true;
	
	if (cleanup)
		delete elt;
		
	return success;
}

/*!
	Returns the pointer to the element right next to the given \a elt.
	
	\sa findPrevMusElement()
*/
CAMusElement *CAStaff::findNextMusElement(CAMusElement *elt) {
	int idx;
	
	for (int i=0; i<_voiceList.size(); i++) {	//go through all the voices and check, if anyone of them includes the given element
		if ((idx = _voiceList[i]->indexOf(elt)) != -1) {
			if (++idx < _voiceList[i]->musElementCount())
				return _voiceList[i]->musElementAt(idx); //return the pointer of the next element
			else
				return 0;	//or 0, if the element is the last one and doesn't have its right neighbour
		}
	}
	
	return 0;	//the element doesn't exist in any of the voices, return 0
}

/*!
	Returns the pointer to the element right before the given \a elt.
	
	\sa findNextMusElement()
*/
CAMusElement *CAStaff::findPrevMusElement(CAMusElement *elt) {
	int idx;
	
	for (int i=0; i<_voiceList.size(); i++) {	//go through all the voices and check, if anyone of them includes the given element
		if ((idx = _voiceList[i]->indexOf(elt)) != -1) {
			if (--idx > -1)
				return _voiceList[i]->musElementAt(idx); //return the pointer of the previous element
			else
				return 0;	//or 0, if the element is the first one and doesn't have its left neighbour
		}
	}
	
	return 0;	//the element doesn't exist in any of the voices, return 0
}

/*!
	Finds the voice names \a name and returns its pointer.
*/
CAVoice *CAStaff::voice(const QString name) {
	for (int i=0; i<_voiceList.size(); i++)
		if (_voiceList[i]->name() == name)
			return _voiceList[i];
	
	return 0;
}

/*!
	Returns a list of pointers to actual music elements which have the given \a startTime and are of
	given \a type.
	This searches the entire staff through all the voices.
	
	\sa CASheet::getChord()
*/
QList<CAMusElement*> CAStaff::getEltByType(CAMusElement::CAMusElementType type, int startTime) {
	QList<CAMusElement*> eltList;
	
	for (int i=0; i<_voiceList.size(); i++) {
		QList<CAMusElement*> curList;
		curList = _voiceList[i]->getEltByType(type, startTime);
		eltList += curList;
	}
	
	return eltList;	
}

/*!
	Checks and fixes, if any of the voices include signs (key sigs, clefs etc.) which aren't
	present in other voices.
	
	Returns true, if everything was ok. False, if fixes were needed.
*/
bool CAStaff::fixVoiceErrors() {
	QList<CAMusElement *> signsNeeded;
	QList<CAMusElement *> prevSignsNeeded;	// list of music elements before that sign in that voice
	QList<CAMusElement *> signsIncluded[voiceCount()];
	
	for (int i=0; i<voiceCount(); i++) {
		QList<CAMusElement*> list = _voiceList[i]->musElementList();
		for (int j=0; j<list.size(); j++) {
			if (!list[j]->isPlayable()) {
				signsIncluded[i] << list[j];	// add the current sign to voice's included list
				if (!signsNeeded.contains(list[j])) {
					signsNeeded << list[j];	// add the current sign to others voices needed list
					prevSignsNeeded << _voiceList[i]->eltBefore(list[j]);
				}
			}
		}
	}
	
	bool everythingIncluded = true;
	for (int i=0; i<signsNeeded.size(); i++) {
		for (int j=0; j<voiceCount(); j++) {
			if (!signsIncluded[j].contains(signsNeeded[i])) {
				everythingIncluded = false;
/*				if (prevSignsNeeded[i] && (!prevSignsNeeded[i]->isPlayable()))
					_voiceList[j]->insertMusElementAfter(signsNeeded[i], prevSignsNeeded[i]);
				else*/
					_voiceList[j]->insertMusElement(signsNeeded[i]);
			}
		}
	}
	
	return everythingIncluded;
}

/*!
	\fn CAStaff::voiceCount()
	Returns the number of voices in the staff.
	
	\sa _voiceList, voice()
*/

/*!
	\fn CAStaff::voiceAt(int i)
	Returns voice with the specified index in the staff starting with 0.
	
	\sa _voiceList, voice()
*/

/*!
	\fn CAStaff::voice(const QString name)
	Looks up for the voice with the given \a name and returns a pointer to it.
	It returns 0, if the voice isn't found.
	
	\sa _voiceList, voiceAt()
*/

/*!
	\fn CAStaff::name()
	Returns the staff name.
	
	\sa _name, setName()
*/

/*!
	\fn CAStaff::setName(QString name)
	Sets the staff name to \a name.
	
	\sa _name, name()
*/

/*!
	\fn CAStaff::_name
	Name of the staff stored in QString format.
	
	\sa name(), setName()
*/
