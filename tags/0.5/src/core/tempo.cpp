/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/tempo.h"

/*!
	\class CATempo
	\brief Tempo mark
	
	Sets and shows the tempo mark.
	It consists of the note (beat) and beats per minute number.
*/

CATempo::CATempo( CAPlayable::CAPlayableLength p, int dotted, int bpm, CAMusElement *t )
 : CAMark( CAMark::Tempo, t ) {
	setBeat( p );
	setBeatDotted( dotted );
	setBpm( bpm );
}

CATempo::~CATempo() {
}

CAMusElement *CATempo::clone() {
	return new CATempo( beat(), beatDotted(), bpm(), associatedElement() );
}

int CATempo::compare( CAMusElement *elt ) {
	if (elt->musElementType()!=CAMusElement::Mark)
		return -2;
	else if (static_cast<CAMark*>(elt)->markType()!=CAMark::Tempo)
		return -1;
	else if (static_cast<CATempo*>(elt)->bpm()!=bpm())
		return 1;
	else if (static_cast<CATempo*>(elt)->beat()!=beat())
		return 2;
	else if (static_cast<CATempo*>(elt)->beatDotted()!=beatDotted())
		return 3;
	else
		return 0;
}
