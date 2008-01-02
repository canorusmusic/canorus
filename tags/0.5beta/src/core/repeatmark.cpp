/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/repeatmark.h"
#include "core/barline.h"

/*!
	\class CARepeatMark
	\brief Repeat marks like segno, volta, coda etc.
	
	This class represents non-ordinary repeat signs like coda, segno and volta.
	
	\sa CABarline
*/

CARepeatMark::CARepeatMark( CABarline *b, CARepeatMarkType t, int voltaNumber )
 : CAMark( CAMark::RepeatMark, b ) {
	setRepeatMarkType( t );
	setVoltaNumber( voltaNumber );
}

CARepeatMark::~CARepeatMark() {
}

CAMusElement *CARepeatMark::clone() {
	return new CARepeatMark( static_cast<CABarline*>(associatedElement()), repeatMarkType(), voltaNumber() );
}

int CARepeatMark::compare( CAMusElement *elt ) {
	if (elt->musElementType()!=CAMusElement::Mark)
		return -2;
	else if (static_cast<CAMark*>(elt)->markType()!=CAMark::RepeatMark)
		return -1;
	else if (static_cast<CARepeatMark*>(elt)->repeatMarkType()!=repeatMarkType())
		return 1;
	else if (static_cast<CARepeatMark*>(elt)->voltaNumber()!=voltaNumber())
		return 2;
	else
		return 0;
}
