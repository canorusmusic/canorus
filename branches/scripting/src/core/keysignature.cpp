/** @file keysignature.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <iostream>	//DEBUG

#include "core/keysignature.h"
#include "core/staff.h"

CAKeySignature::CAKeySignature(CAKeySignatureType type, signed char accs, CADiatonicGenderType gender, CAStaff *staff, int timeStart)
 : CAMusElement(staff, timeStart) {
 	_musElementType = CAMusElement::KeySignature;
 	_keySignatureType = type;
	_diatonicGender = gender;

	for (int i=0; i<7; i++)	//clean up the _accidentals array
		_accidentals[i] = 0;

	//generate the _accidentals array according to the given number of the accidentals
	//eg. _accidentals[3] = -1; means flat on the 3rd note (counting from 0), this means this key signature has Fes instead of F.
	int idx = 3;
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

CAKeySignature::CAKeySignature(QString sPitch, QString sGender, CAStaff *staff, int timeStart)
 : CAMusElement(staff, timeStart) {
	CADiatonicGenderType gender;
	if (sGender.toUpper()=="MAJOR")
		gender = Major;
	else if (sGender.toUpper()=="MINOR")
		gender = Minor;
	else
		gender = (sPitch.at(0).isLower()?Minor:Major);	//if no gender is defined, set the gender according to the pitch capitals
	
	int pitch = sPitch.toUpper()[0].toLatin1() - 'A';

	signed char accs = -1;
	for (int i=5; i!=pitch; i=(i+1)%7)	{//calculate number of accidentals for Major key signatures. Start with F-Major (1 flat) and add 2 sharps mod 7 for every next level
		accs = (accs+2)%7;	
	}
	if (gender == Minor)
		accs -= 3;	//Major->Minor transformation: Every same-name minor key sig has -3 accidentals of the major key signature
	
	while (sPitch.size()>1) {
		if (sPitch.toUpper().endsWith("IS"))
			accs += 7;
		else if (sPitch.toUpper().endsWith("ES"))
			accs -= 7;
		else if (sPitch.toUpper().endsWith("AS"))
			accs -= 7;
		
		sPitch.chop(2);
	}
	
 	//TODO: COPY&PASTE from the first constructor - is there any other way for eg. to call the first constructor from this one? -Matevz
 	_musElementType = CAMusElement::KeySignature;
 	_keySignatureType = Diatonic;
	_diatonicGender = gender;

	for (int i=0; i<7; i++)	//clean up the _accidentals array
		_accidentals[i] = 0;

	//generate the _accidentals array according to the given number of the accidentals
	//eg. _accidentals[3] = -1; means flat on the 3rd note (counting from 0), this means this key signature has Fes instead of F.
	int idx = 3;
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
	signed char sum=0;
	for (int i=0; i<7; i++)
		sum += _accidentals[i];
	
	return sum;
}

const QString CAKeySignature::pitchML() {
	QString ret;
	
	int i;
	int idx=0;
	
	signed char accs = numberOfAccidentals();
	for (i=3; i<accs; i++) {	//start with A and go upwards
		idx = (idx+4)%7;
	}
	for (i=3; i>accs; i--) {	//start with A and go downwards
		idx = (idx+3)%7;
	}
	
	if (_diatonicGender == Minor) {
		idx = (idx+5)%7;
		ret = ('a' + idx);
		if (accs>2)
			ret = "is";
		else if (accs==-6)
			ret = "s";	//es
		else if (accs<-4)
			ret = "es";
	} else {
		ret = ('A' + idx);
		if (accs>5)
			ret += "is";
		else if (accs==-3)
			ret += "s";	//Es
		else if (accs<-3)
			ret += "es";	
	}
	
	return ret;
}

const QString CAKeySignature::diatonicGenderML() {
	switch (_diatonicGender) {
		case Major:
			return QString("major");
			break;
		case Minor:
			return QString("minor");
			break;
	}
	
	return QString();
}
