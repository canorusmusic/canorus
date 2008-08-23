/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <iostream> // debug
#include "core/tuplet.h"
#include "core/playable.h"
#include "core/voice.h"

/*!
	\class CATuplet
	\brief Class used for tuplets (triplets, duols etc.)

	Tuplets are a rhythmic specialty. They are used to shorten the set of
	notes for a specified multiplier.

	Most used tuplets are triplets (multiplier 2/3) and duols (multiplier 3/4).
 */

/*!
	Constructs a tuplet.

	\a number is the existing number of notes and \a actualNumber is the desired
	length of the notes expressed in number of them. These parameters are used to
	calculate the multiplier of the notes.
	eg. number=3, actualNumber=2 multiplies all notes by a factor of 2/3.

	\a noteList is a sorted list of rests and notes under the tuplet. Elements
	should already be part of the voice.
 */
CATuplet::CATuplet( int number, int actualNumber, QList<CAPlayable*> noteList )
 : CAMusElement( noteList.front()->context(), noteList.front()->timeStart(), 0 ), _noteList(noteList), _number(number), _actualNumber(actualNumber) {
	setMusElementType( Tuplet );

	assignTimes();
}

CATuplet::~CATuplet() {
	resetTimes();
}

CAMusElement* CATuplet::clone() {
	CATuplet *t = new CATuplet( number(), actualNumber(), noteList() );
}

int CATuplet::compare(CAMusElement* elt) {
	int diff = 0;
	if ( elt->musElementType()!=CAMusElement::Tuplet ) {
		return -1;
	}

	if ( number() != static_cast<CATuplet*>(elt)->number() )
		diff++;
	if ( actualNumber() != static_cast<CATuplet*>(elt)->actualNumber() )
		diff++;

	return diff;
}

/*!
	Transforms note times to tuplet-affected times.

	The use case should be somewhat this:
	1) Place ordinary notes and rests.
	2) Create a tuplet containing them. This calls assignTimes() automatically
	   to transform music elements times.

	\sa resetTimes()
 */
void CATuplet::assignTimes() {
	CAVoice *voice = noteList().front()->voice();
	CAMusElement *next = 0;
	if ( noteList().back()->musElementType()==Note ) {
		next = voice->next( static_cast<CANote*>(noteList().back())->getChord().back() );
	} else {
		next = voice->next( noteList().back() );
	}

	// removes notes from the voice
	for (int i=noteList().size()-1; i>=0; i--) {
		noteList()[i]->setTuplet(0);
		voice->remove( noteList()[i] );
	}

	for (int i=0; i<noteList().size(); i++) {
		noteList()[i]->setTimeStart( qRound( firstNote()->timeStart() + (noteList()[i]->timeStart() - firstNote()->timeStart()) * ((float)actualNumber() / number()) ) );
	}

	for (int i=0; i<noteList().size()-1; i++) {
		int j=i;
		while ( j<noteList().size() && (noteList()[j]->timeStart() - noteList()[i]->timeStart()) == 0 ) {
			j++;
		}
		if ( j!= noteList().size() ) {
			noteList()[i]->setTimeLength( qRound( noteList()[j]->timeStart() - noteList()[i]->timeStart() ) );
		} else {
			noteList()[i]->setTimeLength( qRound( CAPlayableLength::playableLengthToTimeLength( noteList()[i]->playableLength() ) * ((float)actualNumber() / number()) ) );
		}
	}

	// adds notes back to the voice
	for (int i=0; i<noteList().size(); i++) {
		voice->insert( next, noteList()[i] );
		int j=1;
		for (; i+j<noteList().size() && noteList()[i+j]->timeStart()==noteList()[i]->timeStart(); j++) {
			voice->insert( noteList()[i], noteList()[i+j], true );
		}
		i+=(j-1);
	}

	setTimeLength( noteList().last()->timeEnd()-noteList().front()->timeStart() );

	for (int i=0; i<noteList().size(); i++) {
		noteList()[i]->setTuplet(this);
	}
}

/*!
	Resets the notes times back to their original values before
	placing the tuplet.

	This is usually called from the destructor of the tuplet.
 */
void CATuplet::resetTimes() {
	CAVoice *voice = noteList().front()->voice();
	CAMusElement *next = voice->next( noteList().back() );

	for (int i=0; i<noteList().size(); i++) {
		noteList()[i]->setTuplet( 0 );
	}

	for (int i=0; i<noteList().size(); ) {
		CAPlayable *prevPlayable = (i==0)?(noteList()[i]->voice()->previousPlayable(  noteList()[i]->timeStart() )):(noteList()[i-1]);
		int newTimeStart = (prevPlayable?prevPlayable->timeEnd():0);

		if ( noteList()[i]->musElementType()==Note ) {
			QList<CANote*> chord = static_cast<CANote*>(noteList()[i])->getChord();
			for (int j=0; j<chord.size(); j++, i++) { // chord..
				// removes notes from the voice
				voice->remove( chord[j] );
				chord[j]->calculateTimeLength();
				noteList()[i]->setTimeStart( newTimeStart );
			}
		} else { // rest
			voice->remove( noteList()[i] );
			noteList()[i]->calculateTimeLength();
			noteList()[i]->setTimeStart( newTimeStart );
			i++;
		}
	}

	// adds notes back to the voice
	for (int i=0; i<noteList().size(); i++) {
		voice->insert( next, noteList()[i] );
		int j=1;
		for (; i+j<noteList().size() && noteList()[i+j]->timeStart()==noteList()[i]->timeStart(); j++) { // chord..
			voice->insert( noteList()[i], noteList()[i+j], true );
		}
		i+=(j-1);
	}
}

/*!
	Adds a note to the tuplet.
 */
void CATuplet::addNote( CAPlayable *p ) {
	int i;
	for (i=0; i<noteList().size() && noteList()[i]->timeStart()<p->timeStart(); i++);

	 noteList().insert(i, p);
}
