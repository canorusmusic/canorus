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
#include "core/rest.h" // used for voice synchronization

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
				newStaff->voiceAt(i)->append( newElt,
					voiceAt(i)->musElementAt(eltIdx[i])->musElementType()==CAMusElement::Note &&
					static_cast<CANote*>(voiceAt(i)->musElementAt(eltIdx[i]))->isPartOfTheChord() &&
					!static_cast<CANote*>(voiceAt(i)->musElementAt(eltIdx[i]))->isFirstInTheChord() );
				
				// check tie
				if ( newElt->musElementType()==CAMusElement::Note &&
				     static_cast<CANote*>(voiceAt(i)->musElementAt(eltIdx[i]))->tieStart() ) {
					curTie[i] = static_cast<CANote*>(voiceAt(i)->musElementAt(eltIdx[i]))->tieStart()->clone();
					curTie[i]->setContext( newStaff );
					curTie[i]->setNoteStart( static_cast<CANote*>(newElt) );
					static_cast<CANote*>(newElt)->setTieStart(curTie[i]);
				}
				if ( newElt->musElementType()==CAMusElement::Note &&
				     static_cast<CANote*>(voiceAt(i)->musElementAt(eltIdx[i]))->tieEnd() && curTie[i] ) {
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
			newStaff->voiceAt(i)->synchronizeMusElements();
		}
		
		// append non-playable elements (shared by all voices - only create clone of the first voice element and append it to all)
		if ( eltIdx[0]<voiceAt(0)->musElementCount() ) {
			CAMusElement *newElt = voiceAt(0)->musElementAt(eltIdx[0])->clone();
			newElt->setContext( newStaff );
			
			for (int i=0; i<voiceCount(); i++) {
				newStaff->voiceAt(i)->append( newElt );
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
	Adds an empty voice to the staff and synchronizes it with other voices.
*/  
CAVoice *CAStaff::addVoice() {
	CAVoice *voice = new CAVoice( QObject::tr("Voice%1").arg( voiceCount()+1 ), this );
	addVoice( voice );
	
	return voice;
}

/*!
	Returns the pointer to the element right next to the given \a elt in any of the voice.
	
	\sa next()
*/
CAMusElement *CAStaff::next( CAMusElement *elt ) {
	for ( int i=0; i<voiceCount(); i++ ) {	// go through all the voices and check, if any of them includes the given element
		if ( voiceAt(i)->contains(elt) ) {
			return voiceAt(i)->next(elt);
		}
	}
	
	return 0;	// the element doesn't exist in any of the voices, return 0
}

/*!
	Returns the pointer to the element right before the given \a elt in any of the voice.
	
	\sa previous()
*/
CAMusElement *CAStaff::previous( CAMusElement *elt ) {
	for ( int i=0; i<voiceCount(); i++ ) {	// go through all the voices and check, if any of them includes the given element
		if ( voiceAt(i)->contains(elt) ) {
			return voiceAt(i)->previous(elt);
		}
	}
	
	return 0;	// the element doesn't exist in any of the voices, return 0
}

/*!
	Removes the element \a elt. Updates timeStarts for elements after the given element.
	
	Also updates non-playable shared signs after the element, if \a updateSignTimes is True.
	
	Eventually does the same as CAVoice::remove(), but checks for any voices present in the staff.
*/
bool CAStaff::remove( CAMusElement *elt, bool updateSignTimes ) {
	if ( !elt || !voiceCount() )
		return false;
	
	return voiceAt(0)->remove( elt, updateSignTimes);
}

/*!
	Returns a voice named \a name or Null, if such a voice doesn't exist.
*/
CAVoice *CAStaff::voiceByName(const QString name) {
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
	Returns a list of notes and rests (chord) for all the voices in the staff in
	the given time slice \a time.
	
	This is useful for determination of the harmony at certain point in time.
	
	\sa CASheet:getChord(), CAVoice::getChord()
*/
QList<CAPlayable*> CAStaff::getChord(int time) {
	QList<CAPlayable*> chord;
	for (int i=voiceCount()-1; i>=0; i--)
		chord << voiceAt(i)->getChord(time);
	
	return chord;
}

/*!
	Fixes voices inconsistency:
	1) If any of the voices include signs (key sigs, clefs etc.) which aren't present in all voices,
	   add that sign to all the voices.
	2) If a voice includes a sign which overlaps a playable element in other voice, insert rests until
	   the sign is moved at the end of the overlapped chord in all voices.
	3) If a voice elements are not linear (every N-th element's timeEnd should be N+1-th element's timeStart)
	   inserts rests to achieve linearity.
	
	Synchronizing voices is relatively slow (O(n*m) where n is number of voices and m number of elements
	in voice n). This was the main reason to not automate the synchronization: import filters use lots of
	insertions and synchronization of the voices every time a new element is inserted would considerably
	slow down the import filter.
	
	Returns True, if everything was ok. False, if fixes were needed.
*/
bool CAStaff::synchronizeVoices() {
	int idx[voiceCount()];                    for (int i=0; i<voiceCount(); i++) idx[i]=-1;          // array of current indices of voices at current timeStart
	CAMusElement *lastPlayable[voiceCount()]; for (int i=0; i<voiceCount(); i++) lastPlayable[i]=0;
	
	QList<CAMusElement*> sharedList; // list of shared music elements having the same time-start sorted by voice number
	int timeStart = 0, shortestTime;
	bool done = false;
	bool changesMade = false;
	
	// first fix any inconsistencies inside a voice
	for (int i=0; i<voiceCount(); i++)
		voiceAt(i)->synchronizeMusElements();
	
	while (!done) {
		// gather shared elements into sharedList and remove them from the voice at new timeStart
		for ( int i=0; i<voiceCount(); i++ ) {
			// don't increase idx[i], if the next element is not-playable
			while ( idx[i] < voiceAt(i)->musElementList().size()-1 && !voiceAt(i)->musElementList()[idx[i]+1]->isPlayable() && ( voiceAt(i)->musElementList()[idx[i]+1]->timeStart() == timeStart )) {
				if ( !sharedList.contains(voiceAt(i)->musElementList()[ idx[i]+1 ]) )
					sharedList << voiceAt(i)->musElementList()[ idx[i]+1 ];
				voiceAt(i)->musElementList().removeAt( idx[i]+1 );
			}
		}
		
		// insert all elements from sharedList into all voices
		// OR increase idx[i] for 1 in all voices, if their new element is playable and new timeStart is correct
		if ( sharedList.size() ) {
			for ( int i=0; i<voiceCount(); i++ ) {
				for ( int j=0; j<sharedList.size(); j++) {
					voiceAt(i)->musElementList().insert( idx[i]+1+j, sharedList[j] );
				}
				idx[i]++; // jump to the first one inserted from the sharedList
			}
		} else {
			for ( int i=0; i<voiceCount(); i++ ) {
				if ( idx[i] < voiceAt(i)->musElementList().size()-1 && ( voiceAt(i)->musElementList()[idx[i]+1]->timeStart() == timeStart ) ) {
					if ( voiceAt(i)->musElementList()[idx[i]+1]->isPlayable() ) {
						idx[i]++;
						lastPlayable[i] = voiceAt(i)->musElementList()[idx[i]];
					}
				}
			}
		}
		
		// if the shared element overlaps any of the chords in other voices, insert rests (shift the shared sign forward) to that voice
		for (int i=0; i<voiceCount(); i++) {
			if ( idx[i]==-1 || voiceAt(i)->musElementList()[idx[i]]->isPlayable() ) // only legal idx[i] and non-playable elements
				continue;
			
			for (int j=0; j<voiceCount(); j++) {
				if (i==j) continue;
				
				// fix the overlapped chord, rests are inserted in non-linear part
				if ( idx[j] != -1 && lastPlayable[j] && lastPlayable[j]->timeStart() < timeStart && lastPlayable[j]->timeEnd() > timeStart ) {
					voiceAt(i)->musElementList()[idx[i]]->setTimeStart( lastPlayable[j]->timeEnd() );
					voiceAt(i)->updateTimes( idx[i]+1, lastPlayable[j]->timeEnd() - timeStart, true );
					
					changesMade = true;
				}
			}
		}
		
		// if the elements times are not linear (every N-th element's timeEnd should be N+1-th timeStart), insert rests to achieve it
		for (int j=0; j<voiceCount(); j++) {
			// fix the non-linearity
			if ( idx[j]!=-1 && !voiceAt(j)->musElementList()[idx[j]]->isPlayable() && voiceAt(j)->musElementList()[idx[j]]->timeStart()==timeStart
			     && (lastPlayable[j]?lastPlayable[ j ]->timeEnd():0) < timeStart ) {
				int gapLength = timeStart - ( (idx[j]==-1||!lastPlayable[j])?0:lastPlayable[ j ]->timeEnd() );
				QList<CARest*> restList = CARest::composeRests( gapLength, (idx[j]==-1||!lastPlayable[j])?0:lastPlayable[ j ]->timeEnd(), voiceAt(j) );
				for ( int k=0; k < restList.size(); k++ )
					voiceAt(j)->musElementList().insert( idx[j]++, restList[k] ); // insert the missing rests, rests are added in back, idx++
				voiceAt(j)->updateTimes( idx[j], gapLength, false );              // increase playable timeStarts
				lastPlayable[ j ] = restList.last();
				changesMade = true;
			}
		}
		
		// jump to the last inserted from the sharedList
		if ( sharedList.size() ) {
			for ( int i=0; i<voiceCount(); i++ ) {
				idx[i]+=sharedList.size()-1;
			}
			sharedList.clear();
		}
		
		// shortest time is delta between the current elements and the nearest one in the future
		shortestTime=-1;
		
		for ( int i=0; i<voiceCount(); i++ ) {
			if ( idx[i] < voiceAt(i)->musElementList().size()-1 &&
			     ( shortestTime==-1 ||
			       voiceAt(i)->musElementList()[ idx[i]+1 ]->timeStart() - timeStart < shortestTime )
			   )
				shortestTime = voiceAt(i)->musElementList()[ idx[i]+1 ]->timeStart() - timeStart;
		}
		timeStart += (shortestTime!=-1?shortestTime:0); // increase timeStart
		
		// if all voices are at the end, finish
		done = true;
		for ( int i=0; i<voiceCount(); i++ )
			if ( idx[i] < voiceAt(i)->musElementList().size()-1 )
				done = false;
	}
	
	return changesMade;
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
