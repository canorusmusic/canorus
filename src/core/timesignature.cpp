/* 
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#include "core/timesignature.h"
#include "core/staff.h"

/*!
	\class CATimeSignature
	\brief Represents a time signature in the staff
	
	This class is used for different time signatures in the score.
	Every staff has its own time signature object.
	Time signatures are non-playable objects (timeLength=0) aka signs.
	Time signature object is common to all the voices in one staff
	
	Time signature consists of the upper number - number of beats and the lower number - beat.
	Internal values _beats and _beat represent these values.
	
	\sa CADrawableTimeSignature, CAKeySignature, CAClef, CAStaff
*/

/*!
	\enum CATimeSignature::CATimeSignatureType
	Type of time signature to be shown.
	
	Possible options:
		- Classical
			C for 4/4, C| for 2/2, numbers otherwise. This is default behaviour.
		- Number
			Always show beats/beat, for 4/4 as well
		- Mensural
			Taken from LilyPond. Mensural layout.
		- Neomensural
			Taken from LilyPond. Neomensural layout.
		- Baroque
			Taken from LilyPond. Baroque layout.
	
	\sa timeSignatureType()
*/

/*!
	Creates a time signature with a beat \a beat, number of beats \a beats, parent \a staff, \a
	startTime and of given \a type.
	
	eg. 3/4 time signature should be called new CATimeSignature(3, 4, staff, startTime);
*/
CATimeSignature::CATimeSignature(int beats, int beat, CAStaff *staff, int startTime, CATimeSignatureType type)
 : CAMusElement(staff, startTime) {
 	_musElementType = CAMusElement::TimeSignature;
 	
 	_beats = beats;
 	_beat = beat;
 	_timeSignatureType = type;
}

CATimeSignature::~CATimeSignature() {
}

CATimeSignature *CATimeSignature::clone() {
	return new CATimeSignature(_beats, _beat, (CAStaff*)_context, _timeStart, _timeSignatureType);
}

/*! \deprecated Use timeSignatureTypeToString() instead. This should be moved to LilyPond parser. 	-Matevz
*/
const QString CATimeSignature::timeSignatureTypeML() {
	switch (_timeSignatureType) {
		case Classical:
			return QString("classical");
		case Number:
			return QString("number");
		case Mensural:
			return QString("mensural");
		case Neomensural:
			return QString("neomensural");
		case Baroque:
			return QString("baroque");
	}
}

/*!
	\deprecated New CanorusML parser uses beat and beats directly as integer. The following code
	should be moved to LilyPond parser. -Matevz
*/
const QString CATimeSignature::timeSignatureML() {
	return (QString::number(_beats) + "/" + QString::number(_beat));
}

int CATimeSignature::compare(CAMusElement *elt) {
	if (elt->musElementType()!=CAMusElement::TimeSignature)
		return -1;
	
	int diffs=0;
	if (_timeSignatureType!=((CATimeSignature*)elt)->timeSignatureType()) diffs++;
	if (_beat!=((CATimeSignature*)elt)->beat()) diffs++;
	if (_beats!=((CATimeSignature*)elt)->beats()) diffs++;
	
	return diffs;
}

const QString CATimeSignature::timeSignatureTypeToString(CATimeSignatureType type) {
	switch (type) {
		case Classical:
			return "classical";
		case Number:
			return "number";
		case Mensural:
			return "mensural";
		case Neomensural:
			return "neomensural";
		case Baroque:
			return "baroque";
		default:
			return "";
	}
}

CATimeSignature::CATimeSignatureType CATimeSignature::timeSignatureTypeFromString(const QString type) {
	if (type=="classical") return Classical; else
	if (type=="number") return Number; else
	if (type=="mensural") return Mensural; else
	if (type=="neomensural") return Neomensural; else
	if (type=="baroque") return Baroque;
	else return Classical;
}

/*!
	\fn CATimeSignature::beats()
	Returns the number of beats of this time signature.
	
	\sa setBeats(), _beats, _beat
*/

/*!
	\fn CATimeSignature::setBeats(int beats)
	Sets the number of beats to \a beats for this time signature.
	
	\sa beats(), _beats, _beat
*/

/*!
	\fn CATimeSignature::beat()
	Returns the beat of this time signature.
	
	\sa setBeat(), _beat, _beats
*/

/*!
	\fn CATimeSignature::setBeat(int beat)
	Sets the beat to \a beat for this time signature.
	
	\sa beat(), _beat, _beats
*/

/*!
	\fn CATimeSignature::timeSignatureType()
	Returns type of the time signature.
	
	\sa _timeSignatureType, CATimeSignatureType
*/

/*!
	\fn CATimeSignature::_beats
	Number of beats of this time signature.
	That's the upper number in the time signature.
	
	\sa beats(), setBeats(), _beat
*/

/*!
	\fn CATimeSignature::_beat
	Beat of this time signature.
	That's the lower number in the time signature.
	
	\sa beat(), setBeat(), _beats
*/

/*!
	\fn CATimeSignature::_timeSignatureType
	Stores the type of the time signature.
	
	\sa timeSignatureType(), CATimeSignatureType
*/
