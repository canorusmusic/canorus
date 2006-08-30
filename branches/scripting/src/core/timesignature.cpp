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

const QString CATimeSignature::timeSignatureTypeML() {
	switch (_timeSignatureType) {
		case Classical:
			return QString("classical");
			break;
		case Number:
			return QString("number");
			break;
		case Mensural:
			return QString("mensural");
			break;
		case Neomensural:
			return QString("neomensural");
			break;
		case Baroque:
			return QString("baroque");
			break;		
	}
}
