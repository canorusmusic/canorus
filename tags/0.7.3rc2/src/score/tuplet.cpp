/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <iostream> // debug
#include "score/tuplet.h"
#include "score/playable.h"
#include "score/voice.h"

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
 : CAMusElement( noteList.front()->context(), noteList.front()->timeStart(), 0 ), _number(number), _actualNumber(actualNumber), _noteList(noteList) {
	setMusElementType( Tuplet );

	assignTimes();
}

/*!
	Constructs an empty tuplet.

	Add notes and rests under it manually by calling addNote().
	Call assignTimes() to apply the actual times then.
 */
CATuplet::CATuplet( int number, int actualNumber )
 : CAMusElement( 0, 0, 0 ), _number(number), _actualNumber(actualNumber) {
	setMusElementType( Tuplet );
}

CATuplet::~CATuplet() {
	resetTimes();
}

CATuplet* CATuplet::clone(CAContext* context) { // context is ignored. this method should not be used. FIXME.
	return new CATuplet( number(), actualNumber(), noteList() );
}

CATuplet* CATuplet::clone(QList<CAPlayable*> newList) {
	return new CATuplet( number(), actualNumber(), newList );
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
	Generates a list of pointers to slurs (slur start, slur end, phrasing slur
	start, phrasing slur end) per each note index.
	
	If tuplet contains a rest, no slurs are present at that index.

	\sa assignNoteSlurs()
*/
QList< QList<CASlur*> > CATuplet::getNoteSlurs() {
	QList< QList<CASlur*> > noteSlurs;
	for (int i=0; i<noteList().size(); i++) {
		noteSlurs << QList<CASlur*>();
		if (noteList()[i]->musElementType()==Note) {
			noteSlurs.back() << static_cast<CANote*>(noteList()[i])->slurStart();
			noteSlurs.back() << static_cast<CANote*>(noteList()[i])->slurEnd();
			noteSlurs.back() << static_cast<CANote*>(noteList()[i])->phrasingSlurStart();
			noteSlurs.back() << static_cast<CANote*>(noteList()[i])->phrasingSlurEnd();

			static_cast<CANote*>(noteList()[i])->setSlurStart(0);
			static_cast<CANote*>(noteList()[i])->setSlurEnd(0);
			static_cast<CANote*>(noteList()[i])->setPhrasingSlurStart(0);
			static_cast<CANote*>(noteList()[i])->setPhrasingSlurEnd(0);
		}
	}

	return noteSlurs;
}

/*!
	Assigns the given list of slurs per note index.

	This function is usually called in combination with getNoteSlurs() when
	managing note timeStarts and notes are removed/readded to voices.

	\sa getNoteSlurs()
*/
void CATuplet::assignNoteSlurs( QList< QList<CASlur*> > noteSlurs ) {
	// assign previously remembered slurs
	for (int i=0; i<noteSlurs.size(); i++) {
		if (noteSlurs[i].size()) {
			if (noteSlurs[i][0]) {
				static_cast<CANote*>(noteList()[i])->setSlurStart( noteSlurs[i][0] );
				
				noteSlurs[i][0]->setTimeStart(noteList()[i]->timeStart());
				noteSlurs[i][0]->setTimeLength(noteSlurs[i][0]->timeEnd() - noteList()[i]->timeStart());
			}

			if (noteSlurs[i][1]) {
				static_cast<CANote*>(noteList()[i])->setSlurEnd( noteSlurs[i][1] );
				
				noteSlurs[i][1]->setTimeLength(noteList()[i]->timeStart() - noteSlurs[i][1]->noteStart()->timeStart());
			}

			if (noteSlurs[i][2]) {
				static_cast<CANote*>(noteList()[i])->setPhrasingSlurStart( noteSlurs[i][2] );

				noteSlurs[i][2]->setTimeStart(noteList()[i]->timeStart());
				noteSlurs[i][2]->setTimeLength(noteSlurs[i][2]->timeEnd() - noteList()[i]->timeStart());
			}

			if (noteSlurs[i][3]) {
				static_cast<CANote*>(noteList()[i])->setPhrasingSlurEnd( noteSlurs[i][3] );

				noteSlurs[i][3]->setTimeLength(noteList()[i]->timeStart() - noteSlurs[i][3]->noteStart()->timeStart());
			}
		}
	}
}

/*!
	Transforms note times to tuplet-affected times.

	The use case should be somewhat this:
	1) Place ordinary notes and rests.
	2) Create a tuplet containing them. This calls assignTimes() automatically
	   to transform music elements times.

	This function requires elements to be part of the voice.

	\sa resetTimes()
 */
void CATuplet::assignTimes() {
	resetTimes();

	CAVoice *voice = noteList().front()->voice();
	CAMusElement *next = 0;
	if ( noteList().back()->musElementType()==Note && static_cast<CANote*>(noteList().back())->getChord().size() ) {
		next = voice->next( static_cast<CANote*>(noteList().back())->getChord().back() );
	} else { // rest
		next = voice->next( noteList().back() );
	}

	// removes notes from the voice
	// but first remember the slurs and phrasing slurs
	QList< QList<CASlur*> > noteSlurs = getNoteSlurs();

	for (int i=noteList().size()-1; i>=0; i--) {
		noteList()[i]->setTuplet(0);
		voice->remove( noteList()[i] );
	}

	for (int i=0; i<noteList().size(); i++) {
		noteList()[i]->setTimeStart( qRound( firstNote()->timeStart() + (noteList()[i]->timeStart() - firstNote()->timeStart()) * ((float)actualNumber() / number()) ) );
	}

	for (int i=0; i<noteList().size(); i++) {
		int j=i+1;
		while ( j<noteList().size() && (noteList()[j]->timeStart() == noteList()[i]->timeStart())  ) {
			j++;
		}
		if ( j < noteList().size() ) {
			noteList()[i]->setTimeLength( noteList()[j]->timeStart() - noteList()[i]->timeStart() );
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

	// assign previously remembered slurs
	assignNoteSlurs( noteSlurs );

	setTimeLength( noteList().last()->timeEnd() - noteList().front()->timeStart() );

	for (int i=0; i<noteList().size(); i++) {
		noteList()[i]->setTuplet(this);
	}

	if ( noteList().size() ) {
		setContext( noteList()[0]->context() );
	}
}

/*!
	Resets the notes times back to their original values before
	placing the tuplet.

	This is usually called from the destructor of the tuplet.
 */
void CATuplet::resetTimes() {
	if(noteList().isEmpty())
		return;
	CAVoice *voice = noteList().front()->voice();
	CAMusElement *next = 0;
	if ( noteList().back()->musElementType()==Note && static_cast<CANote*>(noteList().back())->getChord().size() ) {
		next = voice->next( static_cast<CANote*>(noteList().back())->getChord().back() );
	} else { // rest
		next = voice->next( noteList().back() );
	}

	// removes notes from the voice
	// but first remember the slurs and phrasing slurs
	QList< QList<CASlur*> > noteSlurs = getNoteSlurs();

	for (int i=noteList().size()-1; i>=0; i--) {
		noteList()[i]->setTuplet(0);
		voice->remove( noteList()[i] );
	}

	for (int i=0; i<noteList().size(); ) {
		int newTimeStart = (i==0)?(noteList()[0]->timeStart()):(noteList()[i-1]->timeEnd());

		QList<CAPlayable*> chord;
		int j=0;
		for (; i+j<noteList().size() && noteList()[i+j]->timeStart()==noteList()[i]->timeStart(); j++) { // chord..
			chord << noteList()[i+j];
		}

		for (int k=0; k<chord.size(); k++) {
			chord[k]->calculateTimeLength();
			chord[k]->setTimeStart(newTimeStart);
		}

		i+=j;
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
	
	// assign previously remembered slurs
	assignNoteSlurs( noteSlurs );
}

/*!
	Adds a note to the tuplet.
 */
void CATuplet::addNote( CAPlayable *p ) {
	int i;
	for (i=0; i<noteList().size() && noteList()[i]->timeStart() <= p->timeStart() &&
	     (noteList()[i]->musElementType()!=Note || noteList()[i]->timeStart() != p->timeStart() || static_cast<CANote*>(noteList()[i])->diatonicPitch().noteName() < static_cast<CANote*>(p)->diatonicPitch().noteName() ); i++);
	_noteList.insert(i, p);
}

/*!
	Returns a pointer to the next member of tuplet with a greater timeStart.
	If it doesn't exist it returns 0.
 */
CAPlayable* CATuplet::nextTimed( CAPlayable *p ) {
	int t = p->timeStart();
	for (int i=0;i<noteList().size(); i++){
		if( noteList()[i]->timeStart() > t ) return noteList()[i];
	}
	return 0;
}

/*!
	Returns the first note/rest in the first chord of the tuplet.
*/
CAPlayable* CATuplet::firstNote() {
	if (noteList().isEmpty()) return 0;

	if (noteList().first()->musElementType()==CAMusElement::Note &&
			!static_cast<CANote*>(noteList().first())->getChord().isEmpty()) {
		return static_cast<CANote*>(noteList().first())->getChord().first();
	} else {
		return noteList().first();
	}
}

/*!
	Returns the last note/rest in the last chord of the tuplet.
*/
CAPlayable* CATuplet::lastNote() {
	if (noteList().isEmpty()) return 0;

	if (noteList().last()->musElementType()==CAMusElement::Note &&
			!static_cast<CANote*>(noteList().last())->getChord().isEmpty()) {
		return static_cast<CANote*>(noteList().last())->getChord().last();
	} else {
		return noteList().last();
	}
}

int CATuplet::timeLength() {
	if (noteList().size()) {
		return ( noteList().back()->timeEnd() - timeStart() );
	} else {
		return 0;
	}
}

int CATuplet::timeStart() {
	if (noteList().size()) {
		return noteList()[0]->timeStart();
	} else {
		return 0;
	}
}
