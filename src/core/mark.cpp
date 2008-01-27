/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/mark.h"
#include "core/note.h"

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
	setCommon( true );
}

CAMark::CAMark( CAMarkType type, CAContext *c, int timeStart, int timeLength )
 : CAMusElement( c,
                 timeStart,
                 timeLength ) {
	setMusElementType( Mark );	
	setMarkType( type );
	setAssociatedElement( 0 );
	setCommon( true );
}

/*!
	Converts mark type to string.
	
	\sa markTypeFromString()
*/
const QString CAMark::markTypeToString( CAMark::CAMarkType t ) {
	switch (t) {
		case Text:
			return "Text";
		case Tempo:
			return "Tempo";
		case Ritardando:
			return "Ritardando";
		case Dynamic:
			return "Dynamic";
		case Crescendo:
			return "Crescendo";
		case Pedal:
			return "Pedal";
		case InstrumentChange:
			return "InstrumentChange";
		case BookMark:
			return "BookMark";
		case RehersalMark:
			return "RehersalMark";
		case Fermata:
			return "Fermata";
		case RepeatMark:
			return "RepeatMark";
		case Articulation:
			return "Articulation";
		case Fingering:
			return "Fingering";
		default:
			return "Undefined";
	}
}

/*!
	Converts mark type from string.
	
	\sa markTypeToString()
*/
CAMark::CAMarkType CAMark::markTypeFromString( const QString s ) {
	if ( s=="Text" ) {
		return Text;
	} else
	if ( s=="Tempo" ) {
		return Tempo;
	} else
	if ( s=="Ritardando" ) {
		return Ritardando;
	} else
	if ( s=="Dynamic" ) {
		return Dynamic;
	} else
	if ( s=="Crescendo" ) {
		return Crescendo;
	} else
	if ( s=="Pedal" ) {
		return Pedal;
	} else
	if ( s=="InstrumentChange" ) {
		return InstrumentChange;
	} else
	if ( s=="BookMark" ) {
		return BookMark;
	} else
	if ( s=="RehersalMark" ) {
		return RehersalMark;
	} else
	if ( s=="Fermata" ) {
		return Fermata;
	} else
	if ( s=="RepeatMark" ) {
		return RepeatMark;
	} else
	if ( s=="Articulation" ) {
		return Articulation;
	} else
	if ( s=="Fingering" ) {
		return Fingering;
	} else {
		return Undefined;
	}
}

CAMark::~CAMark() {
	if (associatedElement()) {
		associatedElement()->removeMark(this);
	}
}

CAMusElement *CAMark::clone() {
	return new CAMark( markType(), associatedElement(), timeStart(), timeLength() );
}

int CAMark::compare( CAMusElement *elt ) {
	if (elt->musElementType()!=CAMusElement::Mark) {
		return -1;
	} else
	if (static_cast<CAMark*>(elt)->markType()!=markType()) {
		return -1;
	} else {
		return 0;
	}
}

/*!
	\var bool CAMark::_common
	Is mark present in all music elements in the chord. Default: True.
	
	The exception is the fingering which is assigned explicitly to the specific note
	inside the chord
*/
