/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/articulation.h"

/*!
	\class CAArticulation
	\brief Note articulation marks

	Every note can have one or more articulation marks.
	Other music elements can't have articulation marks.

	If the note is part of the chord, all notes in the chord should have
	the same articulation.
*/

CAArticulation::CAArticulation( CAArticulationType t, CANote *n )
 : CAMark( CAMark::Articulation, n ) {
	setArticulationType( t );
}

CAArticulation::~CAArticulation() {
}

CAArticulation* CAArticulation::clone(CAMusElement* elt) {
	return new CAArticulation( articulationType(), (elt->musElementType()==CAMusElement::Note)?static_cast<CANote*>(elt):0 );
}

int CAArticulation::compare(CAMusElement *elt) {
	return 0;
}

const QString CAArticulation::articulationTypeToString( CAArticulationType t ) {
	switch (t) {
	case Accent:
		return "Accent";
	case Marcato:
		return "Marcato";
	case Staccatissimo:
		return "Staccatissimo";
	case Espressivo:
		return "Espressivo";
	case Staccato:
		return "Staccato";
	case Tenuto:
		return "Tenuto";
	case Portato:
		return "Portato";
	case UpBow:
		return "UpBow";
	case DownBow:
		return "DownBow";
	case Flageolet:
		return "Flageolet";
	case Open:
		return "Open";
	case Stopped:
		return "Stopped";
	case Turn:
		return "Turn";
	case ReverseTurn:
		return "ReverseTurn";
	case Trill:
		return "Trill";
	case Prall:
		return "Prall";
	case Mordent:
		return "Mordent";
	case PrallPrall:
		return "PrallPrall";
	case PrallMordent:
		return "PrallMordent";
	case UpPrall:
		return "UpPrall";
	case DownPrall:
		return "DownPrall";
	case UpMordent:
		return "UpMordent";
	case DownMordent:
		return "DownMordent";
	case PrallDown:
		return "PrallDown";
	case PrallUp:
		return "PrallUp";
	case LinePrall:
		return "LinePrall";
	default:
		return "Undefined";
	}
}

CAArticulation::CAArticulationType CAArticulation::articulationTypeFromString( const QString s ) {
	if ( s=="Accent" )
		return Accent;
	else
	if ( s=="Marcato" )
		return Marcato;
	else
	if ( s=="Staccatissimo" )
		return Staccatissimo;
	else
	if ( s=="Espressivo" )
		return Espressivo;
	else
	if ( s=="Staccato" )
		return Staccato;
	else
	if ( s=="Tenuto" )
		return Tenuto;
	else
	if ( s=="Portato" )
		return Portato;
	else
	if ( s=="UpBow" )
		return UpBow;
	else
	if ( s=="DownBow" )
		return DownBow;
	else
	if ( s=="Flageolet" )
		return Flageolet;
	else
	if ( s=="Open" )
		return Open;
	else
	if ( s=="Stopped" )
		return Stopped;
	else
	if ( s=="Turn" )
		return Turn;
	else
	if ( s=="ReverseTurn" )
		return ReverseTurn;
	else
	if ( s=="Trill" )
		return Trill;
	else
	if ( s=="Prall" )
		return Prall;
	else
	if ( s=="Mordent" )
		return Mordent;
	else
	if ( s=="PrallPrall" )
		return PrallPrall;
	else
	if ( s=="PrallMordent" )
		return PrallMordent;
	else
	if ( s=="UpPrall" )
		return UpPrall;
	else
	if ( s=="DownPrall" )
		return DownPrall;
	else
	if ( s=="UpMordent" )
		return UpMordent;
	else
	if ( s=="DownMordent" )
		return DownMordent;
	else
	if ( s=="PrallDown" )
		return PrallDown;
	else
	if ( s=="PrallUp" )
		return PrallUp;
	else
	if ( s=="LinePrall" )
		return LinePrall;
	else
		return Undefined;
}
