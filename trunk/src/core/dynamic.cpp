/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/dynamic.h"
#include "core/note.h"

/*!
	\class CADynamic
	\brief Absolute dynamic marks
	
	Absolute dynamic marks eg. piano, pp, sfz, forte.
*/

CADynamic::CADynamic( QString text, int volume, CANote *note )
 : CAMark( CAMark::Dynamic, note ) {
	setText(text);
	setVolume(volume);
}

CADynamic::~CADynamic() {
}
