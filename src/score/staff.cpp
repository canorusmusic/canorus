/*!
	Copyright (c) 2006-2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QtDebug>

#include <QPainter>
#include <iostream>

#include "score/voice.h"
#include "score/staff.h"
#include "score/note.h"
#include "score/rest.h" // used for voice synchronization
#include "score/tuplet.h"
#include "score/tempo.h"

#include "score/barline.h"
#include "score/timesignature.h"

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
	for (int i=0; i<voiceList().size(); i++) {
		newStaff->addVoice( voiceList()[i]->clone(newStaff) );
	}

	int *peltIdx = new int[voiceList().size()];
	for (int i=0; i<voiceList().size(); i++) peltIdx[i]=0;
	QList<CANote*> tiedOrigNotes; // original notes having opened tie
	QList<CANote*> sluredOrigNotes; // original notes having opened slur
	QList<CANote*> phrasingSluredOrigNotes; // original notes having opened phrasing slur
	QList<CANote*> tiedClonedNotes; // cloned notes having opened tie
	QList<CANote*> sluredClonedNotes; // cloned notes having opened slur
	QList<CANote*> phrasingSluredClonedNotes; // cloned notes having opened phrasing slur

	bool done=false;
	while (!done) {
		// append playable elements
		for (int i=0; i<voiceList().size(); i++) {
			QList<CAPlayable*> elementsUnderTuplet;

			// clone elements in the current voice until the non-playable element is reached
			while ( peltIdx[i]<voiceList()[i]->musElementList().size() && voiceList()[i]->musElementList()[peltIdx[i]]->isPlayable() ) {
				CAPlayable *origElt = static_cast<CAPlayable*>(voiceList()[i]->musElementList()[peltIdx[i]]);
				CAPlayable *clonedElt = origElt->clone( newStaff->voiceList()[i] );
				newStaff->voiceList()[i]->append( clonedElt,
					voiceList()[i]->musElementList()[peltIdx[i]]->musElementType()==CAMusElement::Note &&
					static_cast<CANote*>(origElt)->isPartOfChord() &&
					!static_cast<CANote*>(origElt)->isFirstInChord() );

				if ( origElt->musElementType()==CAMusElement::Note ) {
					CANote *origNote = static_cast<CANote*>(origElt);
					CANote *clonedNote = static_cast<CANote*>(clonedElt);

					// check starting ties, slurs, prasing slurs
					for ( int i=0; i<tiedOrigNotes.size(); i++ ) {
						if ( tiedOrigNotes[i]->tieStart()->noteEnd()==origNote ) {
							CASlur *newTie = tiedOrigNotes[i]->tieStart()->clone(newStaff);
							tiedClonedNotes[i]->setTieStart( newTie );
							newTie->setNoteStart(tiedClonedNotes[i]);
							newTie->setNoteEnd(clonedNote);
							clonedNote->setTieEnd(newTie);

							tiedOrigNotes.removeAt(i);
							tiedClonedNotes.removeAt(i);
						}
					}
					for ( int i=0; i<sluredOrigNotes.size(); i++ ) {
						if ( sluredOrigNotes[i]->slurStart()->noteEnd()==origNote ) {
							CASlur *newSlur = sluredOrigNotes[i]->slurStart()->clone(newStaff);
							sluredClonedNotes[i]->setSlurStart( newSlur );
							newSlur->setNoteStart(sluredClonedNotes[i]);
							newSlur->setNoteEnd(clonedNote);
							clonedNote->setSlurEnd(newSlur);

							sluredOrigNotes.removeAt(i);
							sluredClonedNotes.removeAt(i);
						}
					}
					for ( int i=0; i<phrasingSluredOrigNotes.size(); i++ ) {
						if ( phrasingSluredOrigNotes[i]->phrasingSlurStart()->noteEnd()==origNote ) {
							CASlur *newPhrasingSlur = phrasingSluredOrigNotes[i]->phrasingSlurStart()->clone(newStaff);
							phrasingSluredClonedNotes[i]->setPhrasingSlurStart( newPhrasingSlur );
							newPhrasingSlur->setNoteStart(phrasingSluredClonedNotes[i]);
							newPhrasingSlur->setNoteEnd(clonedNote);
							clonedNote->setPhrasingSlurEnd(newPhrasingSlur);

							phrasingSluredOrigNotes.removeAt(i);
							phrasingSluredClonedNotes.removeAt(i);
						}
					}

					// check ending ties, slurs, phrasing slurs
					if ( origNote->tieStart() ) {
						tiedOrigNotes << origNote;
						tiedClonedNotes << static_cast<CANote*>(clonedElt);
					}
					if ( origNote->slurStart() ) {
						sluredOrigNotes << origNote;
						sluredClonedNotes << static_cast<CANote*>(clonedElt);
					}
					if ( origNote->phrasingSlurStart() ) {
						phrasingSluredOrigNotes << origNote;
						phrasingSluredClonedNotes << static_cast<CANote*>(clonedElt);
					}
				}

				// check tuplets
				if ( origElt->tuplet() ) {
					elementsUnderTuplet << clonedElt;
				}

				if ( origElt->isLastInTuplet() ) {
					new CATuplet( origElt->tuplet()->number(), origElt->tuplet()->actualNumber(), elementsUnderTuplet );
					elementsUnderTuplet.clear();
				}

				peltIdx[i]++;
			}
			newStaff->voiceList()[i]->synchronizeMusElements();
		}

		// append non-playable elements (shared by all voices - only create clone of the first voice element and append it to all)
		if ( peltIdx[0]<voiceList()[0]->musElementList().size() ) {
			CAMusElement *newElt = voiceList()[0]->musElementList()[peltIdx[0]]->clone( newStaff );

			for (int i=0; i<voiceList().size(); i++) {
				newStaff->voiceList()[i]->append( newElt );
				peltIdx[i]++;
			}
		}

		// check if we're at the end
		done = true;
		for (int i=0; i<voiceList().size(); i++) {
			if (peltIdx[i]<voiceList()[i]->musElementList().size()) {
				done = false;
				break;
			}
		}
	}

	delete [] peltIdx;
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
	Adds an empty voice to the staff.
	Call synchronizeVoices() manually to synchronize a new voice with other voices.
*/
CAVoice *CAStaff::addVoice() {
	CAVoice *voice = new CAVoice( name()+QObject::tr("Voice%1").arg( voiceList().size()+1 ), this );
	addVoice( voice );

	return voice;
}

/*!
	Returns the pointer to the element right next to the given \a elt in any of the voice.

	\sa next()
*/
CAMusElement *CAStaff::next( CAMusElement *elt ) {
	for ( int i=0; i<voiceList().size(); i++ ) {	// go through all the voices and check, if any of them includes the given element
		if ( voiceList()[i]->musElementList().contains(elt) ) {
			return voiceList()[i]->next(elt);
		}
	}

	return 0;	// the element doesn't exist in any of the voices, return 0
}

/*!
	Returns the pointer to the element right before the given \a elt in any of the voice.

	\sa previous()
*/
CAMusElement *CAStaff::previous( CAMusElement *elt ) {
	for ( int i=0; i<voiceList().size(); i++ ) {	// go through all the voices and check, if any of them includes the given element
		if ( voiceList()[i]->musElementList().contains(elt) ) {
			return voiceList()[i]->previous(elt);
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
	if ( !elt || !voiceList().size() )
		return false;
	
	return voiceList()[0]->remove( elt, updateSignTimes);
}

/*!
	Returns the first voice with the given \a name or Null, if such a voice doesn't exist.
*/
CAVoice *CAStaff::findVoice(const QString name) {
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
	Returns one music element which has the given \a startTime and \a type.
	This searches through all the voices of the staff.

	\sa CASheet::getChord()
*/
CAMusElement *CAStaff::getOneEltByType(CAMusElement::CAMusElementType type, int startTime) {

	for (int i=0; i<_voiceList.size(); i++) {
		CAMusElement *elt;
		elt = _voiceList[i]->getOneEltByType(type, startTime);
		if (elt) return elt;
	}
	return 0;
}

/*!
	Returns a list of notes and rests (chord) for all the voices in the staff in
	the given time slice \a time.

	This is useful for determination of the harmony at certain point in time.

	\sa CASheet:getChord(), CAVoice::getChord()
*/
QList<CAPlayable*> CAStaff::getChord(int time) {
	QList<CAPlayable*> chord;
	for (int i=voiceList().size()-1; i>=0; i--)
		chord << voiceList()[i]->getChord(time);

	return chord;
}

/*!
	Returns the Tempo element active at the given time.
 */
CATempo *CAStaff::getTempo( int time ) {
	CATempo *tempo = 0;
	for (int i=0; i<voiceList().size(); i++) {
		CATempo *t = voiceList()[i]->getTempo(time);
		if ( t && (!tempo || t->timeStart() > tempo->timeStart()) ) {
			tempo = t;
		}
	}

	return tempo;
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
	int *pidx = new int[voiceList().size()];
        for (int i=0; i<voiceList().size(); i++) pidx[i]=-1;          // array of current indices of voices at current timeStart
	CAMusElement **plastPlayable = new CAMusElement*[voiceList().size()];
 	for (int i=0; i<voiceList().size(); i++) plastPlayable[i]=0;

	_clefList.clear();
	_keySignatureList.clear();
	_timeSignatureList.clear();
	_barlineList.clear();

	int timeStart = 0;
	bool done = false;
	bool changesMade = false;

	// first fix any inconsistencies inside a voice
	for (int i=0; i<voiceList().size(); i++)
		voiceList()[i]->synchronizeMusElements();

	while (!done) {
		QList<CAMusElement*> sharedList; // list of shared music elements having the same time-start sorted by voice number

		// gather shared elements into sharedList and remove them from the voice at new timeStart
		for ( int i=0; i<voiceList().size(); i++ ) {
			// don't increase pidx[i], if the next element is not-playable
			while ( pidx[i] < voiceList()[i]->musElementList().size()-1 && !voiceList()[i]->musElementList()[pidx[i]+1]->isPlayable() && ( voiceList()[i]->musElementList()[pidx[i]+1]->timeStart() == timeStart )) {
				if ( !sharedList.contains(voiceList()[i]->musElementList()[ pidx[i]+1 ]) ) {
					sharedList << voiceList()[i]->musElementList()[ pidx[i]+1 ];
				}
				voiceList()[i]->_musElementList.removeAt( pidx[i]+1 );
			}
		}

		// insert all elements from sharedList into all voices
		// OR increase pidx[i] for 1 in all voices, if their new element is playable and new timeStart is correct
		if ( sharedList.size() ) {
			for ( int i=0; i<voiceList().size(); i++ ) {
				for ( int j=0; j<sharedList.size(); j++) {
					voiceList()[i]->_musElementList.insert( pidx[i]+1+j, sharedList[j] );
				}
				pidx[i]++; // jump to the first one inserted from the sharedList, if inserting shared elts for the first time
				          // or the first one after the sharedList in second pass
			}
			
			// populate the references lists
			for ( int j=0; j<sharedList.size(); j++) {
				switch (sharedList[j]->musElementType()) {
					case CAMusElement::KeySignature: _keySignatureList << sharedList[j]; break;
					case CAMusElement::TimeSignature: _timeSignatureList << sharedList[j]; break;
					case CAMusElement::Clef: _clefList << sharedList[j]; break;
					case CAMusElement::Barline: _barlineList << sharedList[j]; break;
					default: break;
				}
			}

		} else {
			for ( int i=0; i<voiceList().size(); i++ ) {
				if ( pidx[i] < voiceList()[i]->musElementList().size()-1 && ( voiceList()[i]->musElementList()[pidx[i]+1]->timeStart() == timeStart ) ) {
					if ( voiceList()[i]->musElementList()[pidx[i]+1]->isPlayable() ) {
						pidx[i]++;
						plastPlayable[i] = voiceList()[i]->musElementList()[pidx[i]];
					}
				}
			}
		}

		// if the shared element overlaps any of the chords in other voices, insert rests (shift the shared sign forward) to that voice
		for (int i=0; i<voiceList().size(); i++) {
			if ( pidx[i]==-1 || voiceList()[i]->musElementList()[pidx[i]]->isPlayable() ) // only legal pidx[i] and non-playable elements
				continue;

			for (int j=0; j<voiceList().size(); j++) {
				if (i==j) continue;

				// fix the overlapped chord, rests are inserted later in non-linearity check
				if ( pidx[j] != -1 && voiceList()[i]->musElementList()[pidx[i]]->timeStart() == timeStart &&
				     plastPlayable[j] && plastPlayable[j]->timeStart() < timeStart && plastPlayable[j]->timeEnd() > timeStart ) {
					int gapLength = plastPlayable[j]->timeEnd() - timeStart;
					QList<CARest*> restList = CARest::composeRests( gapLength, voiceList()[i]->musElementList()[pidx[i]]->timeStart(), voiceList()[i] );

					voiceList()[i]->musElementList()[pidx[i]]->setTimeStart( plastPlayable[j]->timeEnd() );
					for ( int k=0; k < restList.size(); k++ )
						voiceList()[i]->_musElementList.insert( pidx[i]++, restList[k] ); // insert the missing rests, rests are added in back, pidx++
					voiceList()[i]->updateTimes( pidx[i], gapLength, false );              // increase playable timeStarts
					if (restList.size()) {
						plastPlayable[ i ] = restList.last();
					} else {
						qDebug() << "ERROR CAStaff::synchronizeVoices(): Cannot compose rests of length " << gapLength << endl;
					}

					changesMade = true;
				}
			}
		}

		// if the elements times are not linear (every N-th element's timeEnd should be N+1-th timeStart), insert rests to achieve it
		for (int j=0; j<voiceList().size(); j++) {
			// fix the non-linearity
			if ( pidx[j]!=-1 && !voiceList()[j]->musElementList()[pidx[j]]->isPlayable() && voiceList()[j]->musElementList()[pidx[j]]->timeStart()==timeStart
			     && (plastPlayable[j]?plastPlayable[ j ]->timeEnd():0) < timeStart ) {
				int gapLength = timeStart - ( (pidx[j]==-1||!plastPlayable[j])?0:plastPlayable[ j ]->timeEnd() );
				QList<CARest*> restList = CARest::composeRests( gapLength, (pidx[j]==-1||!plastPlayable[j])?0:plastPlayable[ j ]->timeEnd(), voiceList()[j] );
				for ( int k=0; k < restList.size(); k++ )
					voiceList()[j]->_musElementList.insert( pidx[j]++, restList[k] ); // insert the missing rests, rests are added in back, pidx++
				voiceList()[j]->updateTimes( pidx[j], gapLength, false );              // increase playable timeStarts
				if (restList.size()) {
					plastPlayable[ j ] = restList.last();
				} else {
					qDebug() << "ERROR CAStaff::synchronizeVoices(): Cannot compose rests of length " << gapLength << endl;
				}
				changesMade = true;
			}
		}

		// jump to the last inserted from the sharedList
		if ( sharedList.size() ) {
			for ( int i=0; i<voiceList().size(); i++ ) {
				pidx[i]+=(sharedList.size()-1);
			}
		}

		// shortest time is delta between the current elements and the nearest one in the future
		int shortestTime=-1;

		for ( int i=0; i<voiceList().size(); i++ ) {
			if ( pidx[i] < (voiceList()[i]->musElementList().size()-1) &&
			     ( shortestTime==-1 ||
			       voiceList()[i]->musElementList()[ pidx[i]+1 ]->timeStart() - timeStart < shortestTime )
			   )
				shortestTime = voiceList()[i]->musElementList()[ pidx[i]+1 ]->timeStart() - timeStart;
		}
		int deltaTime = ((shortestTime!=-1)?shortestTime:0);
		timeStart += deltaTime; // increase timeStart

		// if all voices are at the end, finish
		done = (deltaTime==0); // last pass is only meant to linearize
		for ( int i=0; i<voiceList().size(); i++ )
			if ( pidx[i] < voiceList()[i]->musElementList().size()-1 )
				done = false;
	}

	delete [] pidx;
	return changesMade;
}

/*!
	Places a barline in front of the element, if needed and the element is the
	last element in the staff.

	The function finds the last barline and places a new one, if the last bar is full.
	It searches for the time signature in effect for the last bar, not to get fooled by
	time signature(s) already present at a time signature change.

	\return True, if a new barline was placed; otherwise False.
 */
bool CAStaff::placeAutoBar( CAPlayable* elt ) {
	if ( !elt )
		return false;
	CABarline *b = static_cast<CABarline*>(elt->voice()->previousByType( CAMusElement::Barline, elt ));
	CATimeSignature *t;
	CAMusElement *prevTimeSig = elt;

	// do not place autobar, if the element was inserted somewhere in the middle
	for (int i=0; i<elt->voice()->staff()->voiceList().size(); i++) {
		if (elt->voice()->staff()->voiceList()[i]->lastTimeEnd()>elt->timeEnd()) {
			return false;
		}
	}

	do {
		prevTimeSig = elt->voice()->previousByType( CAMusElement::TimeSignature, prevTimeSig );
		t = static_cast<CATimeSignature*>(prevTimeSig);
	} while ( t && prevTimeSig->timeStart() == elt->timeStart() );	// not the time signature for a bar in the future

	if (t) {
		if ( (b?(b->timeStart()):0) + t->barDuration() <= elt->timeStart() ) {
			elt->voice()->insert( elt, new CABarline( CABarline::Single, elt->staff(), elt->timeStart() ) );
			elt->staff()->synchronizeVoices();

			return true;
		}
	}

	return false;
}
