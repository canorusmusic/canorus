/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/tuplet.h"

CATuplet::CATuplet( int number, int actualNumber, QList<CAPlayable*> noteList )
 : _noteList(noteList), _number(number), _actualNumber(actualNumber) {

}

CATuplet::~CATuplet() {
	resetTimes();
}

CAMusElement* CATuplet::clone() {
	CATuplet *t = new CATuplet( number(), actualNumber(), noteList() );
}

int CATuplet::compare(CAMusElement* elt) {
	int diff = 0;
	if ( number() != elt->number() )
		diff++;
	if ( actualNumber() != elt->actualNumber() )
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

		noteList()[i]->setTimeStart( qRound( startNote()->timeStart() + (noteList()[i]->timeStart() - startNote()->timeStart()) * ((float)actualNumber() / number()) ) );
		noteList()[i]->setTimeLength( qRound( noteList()[i]->timeLength() * ((float)actualNumber() / number()) ) );

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
