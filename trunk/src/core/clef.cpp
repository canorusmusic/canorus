/** @file clef.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "core/clef.h"
#include "core/staff.h"

CAClef::CAClef(CAClefType type, CAStaff *staff, int time) : CAMusElement(staff, time) {
	_musElementType = CAMusElement::Clef;
	_clefType = type;
	
	switch (type) {
		case Treble:
			_c1 = -2;
			break;
		case Bass:
			_c1 = 10;
			break;
		case Alto:
			_c1 = 4;
			break;
		case Tenor:
			_c1 = 6;
			break;
		case Soprano:
			_c1 = 0;
			break;
	}
}

const QString CAClef::clefTypeML() {
	QString type;
	
	switch (_clefType) {
		case Treble:
			type = "treble";
			break;
		case Bass:
			type = "bass";
			break;
		case Alto:
			type = "alto";
			break;
		case Tenor:
			type = "tenor";
			break;
		case Soprano:
			type = "soprano";
			break;
	}
	
	return type;
}
