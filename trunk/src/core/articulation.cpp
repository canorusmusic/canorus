/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/articulation.h"

/*!
	\class CAArticulation
	\brief Note articulation marks
	
	Every note can have one or more articulation marks.
	Other music elements can't have articulation marks.
	
	If the note is part of the chord, all notes in the chord should have
	the same articulation. 
*/

CAArticulation::CAArticulation( CAArticulationType t, CANote *n )
 : CAMark( CAMark::Articulation, n ) {
	setArticulationType( t );
}

CAArticulation::~CAArticulation() {
}

CAMusElement* CAArticulation::clone() {
	return new CAArticulation( articulationType(), static_cast<CANote*>(associatedElement()) );
}

int CAArticulation::compare(CAMusElement *elt) {
	return 0;
}
