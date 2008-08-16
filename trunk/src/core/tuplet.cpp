/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/tuplet.h"
#include "core/playable.h"
#include "core/voice.h"

CATuplet::CATuplet( int number, int actualNumber, QList<CAPlayable*> noteList )
 : CAMusElement( noteList.front()->context(), noteList.front()->timeStart(), noteList.last()->timeEnd()-noteList.front()->timeStart() ), _noteList(noteList), _number(number), _actualNumber(actualNumber) {
	setMusElementType( Tuplet );

	CAVoice *voice = noteList.front()->voice();
	CAMusElement *next = voice->next( noteList.back() );

	// removes notes from the voice
	for (int i=0; i<noteList.size(); i++) {
		voice->remove( noteList[i] );
		noteList[i]->setTuplet(this);
	}

	// changes times
	assignTimes();

	// adds notes back to the voice
	for (int i=0; i<noteList.size(); i++) {
		voice->insert( next, noteList[i] );
		int j=1;
		for (; i+j<noteList.size() && noteList[i+j]->timeStart()==noteList[i]->timeStart(); j++) {
			voice->insert( noteList[i], noteList[i+j], true );
		}
		i+=(j-1);
	}
}

CATuplet::~CATuplet() {
	for (int i=0; i<noteList().size(); i++)
		noteList()[i]->setTuplet( 0 );

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
	2) Create a tuplet containing them.
	3) Call assignTimes() to transform music elements times.

	\sa resetTimes()
 */
void CATuplet::assignTimes() {
	for (int i=0; i<noteList().size(); i++) {
		CAMusElement *next = noteList()[i]->voice()->next( noteList()[i] );
		noteList()[i]->voice()->remove( noteList()[i] );

		noteList()[i]->setTimeStart( qRound( firstNote()->timeStart() + (noteList()[i]->timeStart() - firstNote()->timeStart()) * ((float)actualNumber() / number()) ) );
		noteList()[i]->setTimeLength( qRound( CAPlayableLength::playableLengthToTimeLength( noteList()[i]->playableLength() ) * ((float)actualNumber() / number()) ) );

		noteList()[i]->voice()->insert( next, noteList()[i] );
	}
}

/*!
	Resets the notes times back to their original values before
	placing the tuplet.

	This is usually called from the destructor of the tuplet.
 */
void CATuplet::resetTimes() {
	for (int i=0; i<noteList().size(); i++) {
		noteList()[i]->resetTime();
	}
}

/*!
	Adds a note to the tuplet.
 */
void CATuplet::addNote( CAPlayable *p ) {
	int i;
	for (i=0; i<noteList().size() && noteList()[i]->timeStart()<p->timeStart(); i++);

	 _noteList.insert(i, p);
}
