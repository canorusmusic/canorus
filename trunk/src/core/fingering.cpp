/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/fingering.h"
#include "core/note.h"

/*!
	\class CAFingering
	\brief Finger marks
	
	This class represents fingering beside the note.
	The font can be italic (original author's fingering when making an arrangment) or regular.
*/

CAFingering::CAFingering( int finger, CANote *n, bool italic )
 : CAMark( CAMark::Fingering, n ) {
	addFinger( finger );
	setItalic( italic );
}

CAFingering::CAFingering( QList<int> fingers, CANote *n, bool italic )
 : CAMark( CAMark::Fingering, n ) {
	_fingerList = fingers;
	setItalic( italic );
}

CAFingering::~CAFingering() {
}
