/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/crescendo.h"
#include "score/note.h"

/*!
	\class CACrescendo
	\brief Crescendo and Decrescendo marks

	Relative dynamic marks.

	Crescendo starts with the current volume and linearily increases the volume
	to the final volume. Decrescendo decreases volume to the final volume.
*/

CACrescendo::CACrescendo( int volume, CANote *note, CACrescendoType t, int timeStart, int timeLength )
 : CAMark( CAMark::Crescendo, note, timeStart, timeLength ) {
	setFinalVolume(volume);
	setCrescendoType(t);
}

CACrescendo::~CACrescendo() {
}

CACrescendo* CACrescendo::clone(CAMusElement* elt) {
	return new CACrescendo( finalVolume(), (elt->musElementType()==CAMusElement::Note)?static_cast<CANote*>(elt):0, crescendoType(), timeStart(), timeLength() );
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

const QString CACrescendo::crescendoTypeToString( CACrescendoType t ) {
	switch (t) {
	case Crescendo:
		return "Crescendo";
	case Decrescendo:
		return "Decrescendo";
	}
}

CACrescendo::CACrescendoType CACrescendo::crescendoTypeFromString( const QString c ) {
	if (c=="Crescendo") {
		return Crescendo;
	} else
	if (c=="Decrescendo") {
		return Decrescendo;
	}
}
