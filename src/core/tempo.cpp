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
	setPlayableLength( p );
	setDotted( dotted );
	setBpm( bpm );
}

CATempo::~CATempo() {
}
