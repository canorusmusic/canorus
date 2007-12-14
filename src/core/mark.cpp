/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/mark.h"

/*!
	\class CAMark
	\brief Marks that depend on other music elements
	
	This class covers all marks that depend on other music elements. This includes all
	text signs, dynamic marks, articulation, fingering, etc.
*/

CAMark::CAMark( CAMarkType type, CAMusElement *associatedElt, int timeStart, int timeLength )
 : CAMusElement( associatedElt->context(),
                 (timeStart==-1)?associatedElt->timeStart():timeStart,
                 (timeLength==-1)?associatedElt->timeLength():timeLength ) {
	setMusElementType( Mark );	
	setMarkType( type );
	setAssociatedElement( associatedElt );
}

CAMark::CAMark( CAMarkType type, CAContext *c, int timeStart, int timeLength )
 : CAMusElement( c,
                 timeStart,
                 timeLength ) {
	setMusElementType( Mark );	
	setMarkType( type );
	setAssociatedElement( 0 );
}

CAMark::~CAMark() {
}
