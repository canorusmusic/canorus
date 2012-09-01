/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/repeatmark.h"
#include "score/barline.h"

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

CARepeatMark *CARepeatMark::clone(CAMusElement* elt) {
	return new CARepeatMark( (elt->musElementType()==CAMusElement::Barline)?static_cast<CABarline*>(elt):0, repeatMarkType(), voltaNumber() );
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

const QString CARepeatMark::repeatMarkTypeToString( CARepeatMarkType t ) {
	switch (t) {
	case (Undefined):
		return "Undefined";
	case (Volta):
		return "Volta";
	case (Segno):
		return "Segno";
	case (Coda):
		return "Coda";
	case (VarCoda):
		return "VarCoda";
	case (DalSegno):
		return "DalSegno";
	case (DalCoda):
		return "DalCoda";
	case (DalVarCoda):
		return "DalVarCoda";
	}
	return "Undefined";
}

CARepeatMark::CARepeatMarkType CARepeatMark::repeatMarkTypeFromString( const QString r ) {
	if (r=="Undefined")
		return Undefined;
	else
	if (r=="Volta")
		return Volta;
	else
	if (r=="Segno")
		return Segno;
	else
	if (r=="Coda")
		return Coda;
	else
	if (r=="VarCoda")
		return VarCoda;
	else
	if (r=="DalSegno")
		return DalSegno;
	else
	if (r=="DalCoda")
		return DalCoda;
	else
	if (r=="DalVarCoda")
		return DalVarCoda;
	return Undefined;
}

