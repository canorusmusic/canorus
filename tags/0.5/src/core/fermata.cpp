/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/fermata.h"
#include "core/playable.h"
#include "core/barline.h"

/*!
	\class CAFermata
	\brief Fermata mark
	
	Extended mark for fermata.
	Fermata is assigned to playable elements or to the barlines.
*/

CAFermata::CAFermata( CAPlayable *p, CAFermataType t )
 : CAMark( CAMark::Fermata, p ) {
	setFermataType( t );
}

CAFermata::CAFermata( CABarline *b, CAFermataType t )
 : CAMark( CAMark::Fermata, b ) {
	setFermataType( t );
}

CAFermata::~CAFermata() {
}

CAMusElement *CAFermata::clone() {
	if (associatedElement()->isPlayable()) {
		return new CAFermata( static_cast<CAPlayable*>(associatedElement()), fermataType() );
	} else {
		return new CAFermata( static_cast<CABarline*>(associatedElement()), fermataType() );
	}
}

int CAFermata::compare( CAMusElement *elt ) {
	if (elt->musElementType()!=CAMusElement::Mark)
		return -2;
	else if (static_cast<CAMark*>(elt)->markType()!=CAMark::Fermata)
		return -1;
	else if (static_cast<CAFermata*>(elt)->fermataType()!=fermataType())
		return 1;
	
	return 0;
}

const QString CAFermata::fermataTypeToString( CAFermataType t ) {
	switch (t) {
	case NormalFermata:
		return "NormalFermata";
	case ShortFermata:
		return "ShortFermata";
	case LongFermata:
		return "LongFermata";
	case VeryLongFermata:
		return "VeryLongFermata";
	}
}

CAFermata::CAFermataType CAFermata::fermataTypeFromString( const QString r ) {
	if (r=="NormalFermata") {
		return NormalFermata;
	} else
	if (r=="ShortFermata") {
		return ShortFermata;
	} else
	if (r=="LongFermata") {
		return LongFermata;
	} else
	if (r=="VeryLongFermata") {
		return VeryLongFermata;
	}
}
