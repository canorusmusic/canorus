/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPainter>
#include <iostream>
#include "drawable/drawablestaff.h"
#include "drawable/drawableclef.h"
#include "drawable/drawablekeysignature.h"
#include "drawable/drawabletimesignature.h"

#include "core/note.h"
#include "core/clef.h"
#include "core/keysignature.h"
#include "core/timesignature.h"

const float CADrawableStaff::STAFFLINE_WIDTH = 0.8;

CADrawableStaff::CADrawableStaff(CAStaff *s, int x, int y) : CADrawableContext(s, x, y) {
	_drawableContextType = CADrawableContext::DrawableStaff;
	_width = 0;
	_height = 37;
}

void CADrawableStaff::draw(QPainter *p, const CADrawSettings s) {
	QPen pen;
	pen.setWidth((int)(STAFFLINE_WIDTH*s.z));
	pen.setCapStyle(Qt::RoundCap);
	pen.setColor(s.color);
	p->setPen(pen);
	
	int dy=0;
	for (int i=0; i<staff()->numberOfLines(); i++, dy = (int)(((float)i/(staff()->numberOfLines()-1)) * _height * s.z)) {
		p->drawLine(0, s.y + dy,
		            s.w, s.y + dy);
	}
}

CADrawableStaff *CADrawableStaff::clone() {
	CADrawableStaff *d = new CADrawableStaff(staff(), _xPos, _yPos);
	
	return d;
}

/*!
	Returns the center Y coordinate of the given note in this staff.
	
	\param pitch Note pitch which the following coordinates are being calculated for.
	\param clef Corresponding clef.
	\return Center of a space/line of a staff in absolute world units.
*/
int CADrawableStaff::calculateCenterYCoord(int pitch, CAClef *clef) {
	return (int)( yPos() + height() -
	               //middle c in logical pitch is 28
	               ((pitch - 28) + (clef?clef->c1():-2) + 0.0)*(lineSpace()/2)
	            );
}

/*!
	This is an overloaded member function, provided for convenience.
	
	Returns the center Y coordinate of the given note in this staff.
	
	\param note Note which the following coordinates are being calculated.
	\param x X coordinate of the note.
	\return Center of a space/line of a staff in absolute world units.
*/
int CADrawableStaff::calculateCenterYCoord(CANote *note, int x) {
	CAClef *clef = getClef(x);
	return calculateCenterYCoord(note->pitch(), clef);
}

/*!
	This is an overloaded member function, provided for convenience.
	
	Returns the center Y coordinate of the given note in this staff.
	
	\param pitch Note pitch which the following coordinates are being calculated for.
	\param x X coordinate of the note.
	\return Center of a space/line of a staff in absolute world units.
*/
int CADrawableStaff::calculateCenterYCoord(int pitch, int x) {
	CAClef *clef = getClef(x);
	return calculateCenterYCoord(pitch, clef);
}

/*!
	This is an overloaded member function, provided for convenience.
	
	Returns the center Y coordinate of the given note in this staff.
	
	\param note Note which the following coordinates are being calculated.
	\param clef Corresponding clef.
	\return Center of a space/line of a staff in absolute world units.
*/
int CADrawableStaff::calculateCenterYCoord(CANote *note, CAClef *clef) {
	return calculateCenterYCoord(note->pitch(), clef);
}

/*!
	Rounds the given Y coordinate to the nearest one so it fits a line or a space (ledger lines too, if needed) in a staff.
	
	\return Center of the nearest space/line of a staff, whichever is closer in absolute world units.
*/
int CADrawableStaff::calculateCenterYCoord(int y) {
	float newY = (y - yPos()) / (lineSpace()/2);
	newY += 0.5*((y-yPos()<0)?-1:1);	//round
	newY = (float)((int)newY);
	
	return (int)(yPos() + ((newY) * (lineSpace()/2)));
}

/*!
	Calculates the note pitch on the given clef and absolute world Y coordinate.
	
	\param x X coordinate in absolute world units.
	\param y Y coordinate in absolute world units.
	\return Note pitch in logical units. 
*/
int CADrawableStaff::calculatePitch(int x, int y) {
	CAClef *clef = getClef(x);
	float yC1 = yPos() + height() - (clef?clef->c1():-2)*(lineSpace()/2); // Y coordinate of c1 of the current staff
	
	// middle c = 28
	return qRound( 28 - (y - yC1)/(lineSpace()/2.0) );
}

/*!
	Adds a clef \a clef to the clef list for faster search of the current clef in the staff.
*/
void CADrawableStaff::addClef(CADrawableClef *clef) {
	int i;
	for (i=0; ((i<_drawableClefList.size()) && (clef->xPos() > _drawableClefList[i]->xPos())); i++);
	_drawableClefList.insert(i, clef);
}

/*!
	Removes the given clef from the clefs-lookup list.
	Returns True, if the clef was successfully removed, False otherwise.
*/
bool CADrawableStaff::removeClef(CADrawableClef *clef) {
	return _drawableClefList.removeAll(clef);
}

/*!
	Returns the pointer to the last clef placed before the given X-coordinate.
*/
CAClef* CADrawableStaff::getClef(int x) {
	int i;
	for (i=0; ((i<_drawableClefList.size()) && (x > _drawableClefList[i]->xPos())); i++);
	
	return ((--i<0)?0:_drawableClefList[i]->clef());
}

/*!
	Returns accidentals at the given X-coordinate and pitch. eg. -1 for one flat, 2 for two sharps.
	This is useful to determine the note's pitch to be placed in certain measure or part of the measure,
	if accidentals have been placed before.
*/
int CADrawableStaff::getAccs(int x, int pitch) {
	CAKeySignature *key = getKeySignature(x);
	
	//find nearest left element
	int i; for (i=0; i<_drawableMusElementList.size() && _drawableMusElementList[i]->xPos() < x; i++); i--;
	
	while (i>=0 &&
	       _drawableMusElementList[i]->drawableMusElementType() != CADrawableMusElement::DrawableBarline &&
	       _drawableMusElementList[i]->drawableMusElementType() != CADrawableMusElement::DrawableKeySignature &&
	       (!(_drawableMusElementList[i]->drawableMusElementType() == CADrawableMusElement::DrawableNote && 
	          ((CANote*)_drawableMusElementList[i]->musElement())->pitch() == pitch
	         ))
	      ) {	//go back while a barline, key signature or a note with accidental is found
	      	i--;
	}
	
	if (i==-1)
		return 0;
	if (_drawableMusElementList[i]->drawableMusElementType() == CADrawableMusElement::DrawableBarline ||
	    _drawableMusElementList[i]->drawableMusElementType() == CADrawableMusElement::DrawableKeySignature)
		return (key?key->accidentals()[pitch%7]:0);
	else //note before
		return ((CANote*)_drawableMusElementList[i]->musElement())->accidentals();
}

/*!
	Adds a key signature \a keySig to the key signatures list for faster search of the current key signature in the staff.
*/
void CADrawableStaff::addKeySignature(CADrawableKeySignature *keySig) {
	int i;
	for (i=0; ((i<_drawableKeySignatureList.size()) && (keySig->xPos() > _drawableKeySignatureList[i]->xPos())); i++);
	_drawableKeySignatureList.insert(i, keySig);
}

/*!
	Removes the given key signature from the key signatures-lookup list.
	Returns True, if the key signature was successfully removed, False otherwise.
*/
bool CADrawableStaff::removeKeySignature(CADrawableKeySignature *keySig) {
	return _drawableKeySignatureList.removeAll(keySig);
}

/*
	Returns the pointer to the last key signature placed before the given X-coordinate.
*/
CAKeySignature* CADrawableStaff::getKeySignature(int x) {
	int i;
	for (i=0; ((i<_drawableKeySignatureList.size()) && (x > _drawableKeySignatureList[i]->xPos())); i++);
	
	return ((--i<0)?0:_drawableKeySignatureList[i]->keySignature());
}

/*!
	Adds a time signature \a timeSig to the time signatures list for faster search of the current time signature in the staff.
*/
void CADrawableStaff::addTimeSignature(CADrawableTimeSignature *timeSig) {
	int i;
	for (i=0; ((i<_drawableTimeSignatureList.size()) && (timeSig->xPos() > _drawableTimeSignatureList[i]->xPos())); i++);
	_drawableTimeSignatureList.insert(i, timeSig);
}

/*!
	Removes the given time signature from the time signatures-lookup list.
	Returns True, if the time signature was successfully removed, False otherwise.
*/
bool CADrawableStaff::removeTimeSignature(CADrawableTimeSignature *timeSig) {
	return _drawableTimeSignatureList.removeAll(timeSig);
}

/*!
	Returns the pointer to the last time signature placed before the given X-coordinate.
*/
CATimeSignature* CADrawableStaff::getTimeSignature(int x) {
	int i;
	for (i=0; ((i<_drawableTimeSignatureList.size()) && (x > _drawableTimeSignatureList[i]->xPos())); i++);
	
	return ((--i<0)?0:_drawableTimeSignatureList[i]->timeSignature());
}

void CADrawableStaff::addMElement(CADrawableMusElement *elt) {
	switch (elt->drawableMusElementType()) {
		case CADrawableMusElement::DrawableClef:
			addClef((CADrawableClef*)elt);
			break;
		case CADrawableMusElement::DrawableKeySignature:
			addKeySignature((CADrawableKeySignature*)elt);
			break;
		case CADrawableMusElement::DrawableTimeSignature:
			addTimeSignature((CADrawableTimeSignature*)elt);
			break;
	}
	
	_drawableMusElementList << elt;
}

int CADrawableStaff::removeMElement(CADrawableMusElement *elt) {
	switch (elt->drawableMusElementType()) {
		case CADrawableMusElement::DrawableClef:
			removeClef((CADrawableClef*)elt);
			break;
		case CADrawableMusElement::DrawableKeySignature:
			removeKeySignature((CADrawableKeySignature*)elt);
			break;
		case CADrawableMusElement::DrawableTimeSignature:
			removeTimeSignature((CADrawableTimeSignature*)elt);
			break;
	}
	
	return _drawableMusElementList.removeAll(elt);
}
