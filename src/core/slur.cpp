/* 
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#include "core/slur.h"
#include "core/note.h"

/*!
	\class CASlur
	\brief Slur, Tie, Phrasing slur and Laissez vibrer tie
	This class represents any type of the slur.
	It doesn't have any references to notes it represents, but notes have pointers to
	slurs if the note has one.
	timeStart is the timeStart of the first note.
	timeLength is the delta between first and second notes timeStarts.
	
	\sa CANote::_slurStart, \sa CANote::_tieStart, \sa CANote::_phrasingSlurStart
*/

/*!
	\enum CASlur::CASlurDirection
	\brief Direction of the slur
	
	This type represents the direction of the note's slur.
	
	Possible values:
		- SlurUp
			Slur high point above the left and right points.
		- SlurDown
			Slur high point below the left and right points.
*/

/*!
	Default constructor.
*/
CASlur::CASlur( CASlurType type, CASlurDirection dir, CAContext *c, CANote *noteStart, CANote *noteEnd )
 : CAMusElement( c, noteStart->timeStart(), 0 ) {
	setSlurDirection( dir );
	setSlurType( type );
	
	setNoteStart( noteStart );
	setNoteEnd( noteEnd );
	
	if (noteEnd)
		setTimeLength( noteEnd->timeStart() - noteStart->timeStart() );
	else
		setTimeLength( noteStart->timeLength() );
}

CASlur::~CASlur() {
	switch (slurType()) {
		case Tie:
			if ( noteStart() ) noteStart()->setTieStart( 0 );
			if ( noteEnd() ) noteEnd()->setTieEnd( 0 );
			break;
		case Slur:
			if ( noteStart() ) noteStart()->setSlurStart( 0 );
			if ( noteEnd() ) noteEnd()->setSlurEnd( 0 );
			break;
		case PhrasingSlur:
			if ( noteStart() ) noteStart()->setPhrasingSlurStart( 0 );
			if ( noteEnd() ) noteEnd()->setPhrasingSlurEnd( 0 );
			break;
	}
}

CASlur *CASlur::clone() {
}

int CASlur::compare( CAMusElement *elt ) {
	if (elt->musElementType()!=CAMusElement::Slur)
		return -1;
	
	int diffs=0;
	if ( slurDirection() != static_cast<CASlur*>(elt)->slurDirection() ) diffs++;
	
	return diffs;
}
