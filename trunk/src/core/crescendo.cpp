/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/crescendo.h"
#include "core/note.h"

/*!
	\class CACrescendo
	\brief Crescendo and Decrescendo marks
	
	Relative dynamic marks.
	
	Crescendo starts with the current volume and linearily increases the volume
	to the final volume. Decrescendo decreases volume to the final volume.
*/

CACrescendo::CACrescendo( int volume, CANote *note, CACrescendoType t )
 : CAMark( CAMark::Crescendo, note ) {
	setFinalVolume(volume);
	setCrescendoType(t);
}

CACrescendo::~CACrescendo() {
}

CAMusElement* CACrescendo::clone() {
	return new CACrescendo( finalVolume(), static_cast<CANote*>(associatedElement()), crescendoType() );
}

int CACrescendo::compare( CAMusElement *elt ) {
	if (elt->musElementType()!=CAMusElement::Mark)
		return -2;
	
	if (static_cast<CAMark*>(elt)->markType()!=CAMark::Crescendo)
		return -1;
	
	if (static_cast<CACrescendo*>(elt)->finalVolume()!=finalVolume())
		return 1;
	
	if (static_cast<CACrescendo*>(elt)->crescendoType()!=crescendoType())
		return 1;
	
	return 0;
}
