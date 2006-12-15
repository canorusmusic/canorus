/** @file timesignature.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "core/timesignature.h"
#include "core/staff.h"

CATimeSignature::CATimeSignature(int beats, int beat, CAStaff *staff, int startTime, CATimeSignatureType type)
 : CAMusElement(staff, startTime) {
 	_musElementType = CAMusElement::TimeSignature;
 	
 	_beats = beats;
 	_beat = beat;
 	_timeSignatureType = type;
}

CATimeSignature::CATimeSignature(QString MLString, CAStaff *staff, int startTime, QString type)
 : CAMusElement(staff, startTime) {
 	_musElementType = CAMusElement::TimeSignature;
 	
 	_beats = MLString.mid(0, MLString.indexOf("/")).toInt();
 	_beat = MLString.mid(MLString.indexOf("/") + 1).toInt();
 	
 	if (type=="number") 
 		_timeSignatureType = Number;
 	else if (type=="mensural") 
 		_timeSignatureType = Mensural;
 	else if (type=="neomensural") 
 		_timeSignatureType = Neomensural;
 	else if (type=="baroque") 
 		_timeSignatureType = Baroque;
 	else
 		_timeSignatureType = Classical;
}

CATimeSignature::~CATimeSignature() {
}

CATimeSignature *CATimeSignature::clone() {
	return new CATimeSignature(_beats, _beat, (CAStaff*)_context, _timeStart, _timeSignatureType);
}

/** OBSOLETE */
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
