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

int CADrawableStaff::calculateCenterYCoord(int pitch, CAClef *clef) {
	return (int)( yPos() + height() -
	               //middle c in logical pitch is 28
	               ((pitch - 28) + (clef?clef->c1():-2) + 0.0)*(lineSpace()/2)
	            );
}

int CADrawableStaff::calculateCenterYCoord(CANote *note, int x) {
	CAClef *clef = getClef(x);
	return calculateCenterYCoord(note->pitch(), clef);
}

int CADrawableStaff::calculateCenterYCoord(int pitch, int x) {
	CAClef *clef = getClef(x);
	return calculateCenterYCoord(pitch, clef);
}

int CADrawableStaff::calculateCenterYCoord(CANote *note, CAClef *clef) {
	return calculateCenterYCoord(note->pitch(), clef);
}

int CADrawableStaff::calculateCenterYCoord(int y) {
	float newY = (y - yPos()) / (lineSpace()/2);
	newY += 0.5*((y-yPos()<0)?-1:1);	//round
	newY = (float)((int)newY);
	
	return (int)(yPos() + ((newY) * (lineSpace()/2)));
}

int CADrawableStaff::calculatePitch(int x, int y) {
	CAClef *clef = getClef(x);
	float yC1 = yPos() + height() - (clef?clef->c1():-2)*(lineSpace()/2); // Y coordinate of c1 of the current staff
	
	// middle c = 28
	return qRound( 28 - (y - yC1)/(lineSpace()/2.0) );
}

void CADrawableStaff::addClef(CADrawableClef *clef) {
	int i;
	for (i=0; ((i<_drawableClefList.size()) && (clef->xPos() > _drawableClefList[i]->xPos())); i++);
	_drawableClefList.insert(i, clef);
}

bool CADrawableStaff::removeClef(CADrawableClef *clef) {
	return _drawableClefList.removeAll(clef);
}

CAClef* CADrawableStaff::getClef(int x) {
	int i;
	for (i=0; ((i<_drawableClefList.size()) && (x > _drawableClefList[i]->xPos())); i++);
	
	return ((--i<0)?0:_drawableClefList[i]->clef());
}

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

void CADrawableStaff::addKeySignature(CADrawableKeySignature *keySig) {
	int i;
	for (i=0; ((i<_drawableKeySignatureList.size()) && (keySig->xPos() > _drawableKeySignatureList[i]->xPos())); i++);
	_drawableKeySignatureList.insert(i, keySig);
}

bool CADrawableStaff::removeKeySignature(CADrawableKeySignature *keySig) {
	return _drawableKeySignatureList.removeAll(keySig);
}

CAKeySignature* CADrawableStaff::getKeySignature(int x) {
	int i;
	for (i=0; ((i<_drawableKeySignatureList.size()) && (x > _drawableKeySignatureList[i]->xPos())); i++);
	
	return ((--i<0)?0:_drawableKeySignatureList[i]->keySignature());
}

void CADrawableStaff::addTimeSignature(CADrawableTimeSignature *timeSig) {
	int i;
	for (i=0; ((i<_drawableTimeSignatureList.size()) && (timeSig->xPos() > _drawableTimeSignatureList[i]->xPos())); i++);
	_drawableTimeSignatureList.insert(i, timeSig);
}

bool CADrawableStaff::removeTimeSignature(CADrawableTimeSignature *timeSig) {
	return _drawableTimeSignatureList.removeAll(timeSig);
}

CATimeSignature* CADrawableStaff::getTimeSignature(int x) {
	int i;
	for (i=0; ((i<_drawableTimeSignatureList.size()) && (x > _drawableTimeSignatureList[i]->xPos())); i++);
	
	return ((--i<0)?0:_drawableTimeSignatureList[i]->timeSignature());
}

int CADrawableStaff::calculateHighestCenterYCoord(int pitch, int x) {
	CAClef *clef = getClef(x);
	
	int line = clef?clef->c1():-2;
	line -= 7;
	
	while ((line + pitch + 7) < (staff()->numberOfLines()*2))	//while the height still doesn't reach the upper ledger lines
		line += 7;
	
	return (int)(yPos() + height() - (lineSpace()/2) * (line+pitch));
}

int CADrawableStaff::calculateLowestCenterYCoord(int pitch, int x) {
	CAClef *clef = getClef(x);
	
	int line = clef?clef->c1():-2;
	line += 7;
	
	while ((line + pitch - 7) > -2)	//while the depth still doesn't reach the lower ledger lines
		line -= 7;
	
	return (int)(yPos() + height() - (lineSpace()/2) * (line+pitch));
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
