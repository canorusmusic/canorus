/** @file keysignature.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "core/keysignature.h"
#include "core/staff.h"

CAKeySignature::CAKeySignature(CAKeySignatureType type, signed char accs, CAStaff *staff, int timeStart)
 : CAMusElement(staff, timeStart) {
 	_musElementType = CAMusElement::KeySignature;
 	_keySignatureType = type;

	for (int i=0; i<7; i++)	//clean up the _accidentals array
		_accidentals[i] = 0;

	//generate the _accidentals array according to the given number of the accidentals
	//eg. _accidentals[3] = -1; means flat on the 3rd note (counting from 0), this means this key signature has Fes instead of F.
	int idx;
	
	idx = 3;
	for (int i=0; i<accs; i++) {	//key signatures with sharps
		_accidentals[idx] = 1;
		idx = (idx+4)%7;	//the circle of fifths in positive direction - add a Fifth
	}
	
	idx = 6;
	for (int i=0; i>accs; i--) {	//key signatures with flats
		_accidentals[idx] = -1;
		idx = (idx+3)%7;	//the circle of fifths in negative direction - add a Fourth
	}
}

CAKeySignature::~CAKeySignature() {
	
}

signed char CAKeySignature::numberOfAccidentals() {
	signed char sum;
	for (int i=0; i<7; i++)
		sum += _accidentals[i];
	
	return sum;
}
