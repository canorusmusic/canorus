/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/slur.h"
#include "score/note.h"

/*!
	\class CASlur
	\brief Slur, Tie, Phrasing slur and Laissez vibrer tie
	This class represents any type of slur.
	Holds pointers to the first and last notes.

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
		- SlurNeutral
			Slur on the oposite side of the stem.
		- SlurPreferred
			Slur direction determined by CANote::determineSlurDirection() (default)
*/

/*!
	Default constructor.
*/
CASlur::CASlur( CASlurType type, CASlurDirection dir, CAContext *c, CANote *noteStart, CANote *noteEnd, CASlurStyle style )
 : CAMusElement( c, noteStart->timeStart(), 0 ) {
 	setMusElementType( CAMusElement::Slur );
	setSlurDirection( dir );
	setSlurType( type );
	setSlurStyle( style );

	setNoteStart( noteStart );
	setNoteEnd( noteEnd );

	if (noteEnd)
		setTimeLength( noteEnd->timeStart() - noteStart->timeStart() );
	else
		setTimeLength( noteStart->timeLength() );
}

CASlur::~CASlur() {
	switch (slurType()) {
		case TieType:
			if ( noteStart() ) noteStart()->setTieStart( nullptr );
			if ( noteEnd() ) noteEnd()->setTieEnd( nullptr );
			break;
		case SlurType:
			if ( noteStart() ) noteStart()->setSlurStart( nullptr );
			if ( noteEnd() ) noteEnd()->setSlurEnd( nullptr );
			break;
		case PhrasingSlurType:
			if ( noteStart() ) noteStart()->setPhrasingSlurStart( nullptr );
			if ( noteEnd() ) noteEnd()->setPhrasingSlurEnd( nullptr );
			break;
	}
}

// FIXME copying noteStart/End pointers.
CASlur *CASlur::clone(CAContext* context) {
	return new CASlur( slurType(), slurDirection(), context, noteStart(), noteEnd(), slurStyle() );
}

CASlur *CASlur::clone(CAContext* context, CANote* noteStart, CANote* noteEnd) {
	return new CASlur( slurType(), slurDirection(), context, noteStart, noteEnd, slurStyle() );
}

int CASlur::compare( CAMusElement *elt ) {
	if (elt->musElementType()!=CAMusElement::Slur)
		return -1;

	int diffs=0;
	if ( slurDirection() != static_cast<CASlur*>(elt)->slurDirection() ) diffs++;

	return diffs;
}

const QString CASlur::slurStyleToString( CASlur::CASlurStyle style ) {
	switch (style) {
		case SlurSolid:
			return "slur-solid";
		case SlurDotted:
			return "slur-dotted";
		case Undefined:
			break;
	}

	return "";
}

CASlur::CASlurStyle CASlur::slurStyleFromString( const QString style ) {
	if ( style=="slur-solid" )
		return SlurSolid;
	else if ( style == "slur-dotted" )
		return SlurDotted;

	return Undefined;
}

const QString CASlur::slurDirectionToString( CASlur::CASlurDirection dir ) {
	switch (dir) {
		case SlurUp:
			return "slur-up";
		case SlurDown:
			return "slur-down";
		case SlurNeutral:
			return "slur-neutral";
		case SlurPreferred:
			return "slur-preferred";
	}

	return "";
}

CASlur::CASlurDirection CASlur::slurDirectionFromString( const QString dir ) {
	if ( dir=="slur-up" )
		return SlurUp;
	else if ( dir == "slur-down" )
		return SlurDown;
	else if ( dir == "slur-neutral" )
		return SlurNeutral;
	else if ( dir == "slur-pereferred" )
		return SlurPreferred;

	return SlurPreferred;
}
