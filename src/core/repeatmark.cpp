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

CARepeatMark::CARepeatMark( CABarline *b, CARepeatMarkType t )
 : CAMark( CAMark::RepeatMark, b ) {
	setRepeatMarkType( t );
}

CARepeatMark::~CARepeatMark() {
}
