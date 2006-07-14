/** @file clef.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "clef.h"
#include "staff.h"

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
