/** @file drawablestaff.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QPainter>
#include <iostream>
#include "drawablestaff.h"
#include "drawableclef.h"
#include "staff.h"
#include "note.h"
#include "clef.h"

#define _lineSpace (staff()->numberOfLines()?(float)_height/(staff()->numberOfLines()-1):0)

CADrawableStaff::CADrawableStaff(CAStaff *s, int x, int y) : CADrawableContext(s, x, y) {
	_drawableContextType = CADrawableContext::DrawableStaff;
	_width = 0;
	_height = 35;
}

void CADrawableStaff::draw(QPainter *p, const CADrawSettings s) {
	QPen pen;
	pen.setWidth((int)(0.8*s.z));
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

int CADrawableStaff::calculateCenterYCoord(CANote *note, CAClef *clef) {
	return (int)( yPos() + height() -
	               //middle c in logical pitch is 28
	               ((note->pitch() - 28) + (clef?clef->c1():-2) + 0.0)*(_lineSpace/2)
	            );
}

int CADrawableStaff::calculateCenterYCoord(CANote *note, int x) {
	CAClef *clef = getClef(x);
	return calculateCenterYCoord(note, clef);
}

int CADrawableStaff::calculateCenterYCoord(int y) {
	float newY = (y - yPos()) / (_lineSpace/2);
	newY += 0.5*((y-yPos()<0)?-1:1);	//round
	newY = (float)((int)newY);	// "

	return (int)(yPos() + ((newY) * (_lineSpace/2)));
}

int CADrawableStaff::calculatePitch(int x, int y) {
	CAClef *clef = getClef(x);
	float yC1 = yPos() + height() - (clef?clef->c1():-2)*(_lineSpace/2); //Y coordinate of c1 of the current staff

	//middle c = 28
	return (int)(28 - (y - yC1)/(_lineSpace/2) + 0.5);
}

void CADrawableStaff::addClef(CADrawableClef *clef) {
	int i;
	for (i=0; ((i<_drawableClefList.size()) && (clef->xPos() < _drawableClefList[i]->xPos())); i++);
	_drawableClefList.insert(i, clef);
}

CAClef* CADrawableStaff::getClef(int x) {
	int i;
	for (i=0; ((i<_drawableClefList.size()) && (x > _drawableClefList[i]->xPos())); i++);
	
	return (--i<0?0:_drawableClefList[i]->clef());
}
