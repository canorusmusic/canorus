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
		case Crescendo:
			return "Crescendo";
		case Dynamic:
			return "Dynamic";
		case Pedal:
			return "Pedal";
		case InstrumentChange:
			return "InstrumentChange";
		case Bookmark:
			return "Bookmark";
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
	if ( s=="Crescendo" ) {
		return Crescendo;
	} else
	if ( s=="Dynamic" ) {
		return Dynamic;
	} else
	if ( s=="Pedal" ) {
		return Pedal;
	} else
	if ( s=="InstrumentChange" ) {
		return InstrumentChange;
	} else
	if ( s=="Bookmark" ) {
		return Bookmark;
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
}
