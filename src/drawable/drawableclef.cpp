/** @file drawableclef.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <iostream>

#include <QFont>
#include <QPainter>

#include "drawable/drawableclef.h"
#include "drawable/drawablestaff.h"

#include "core/clef.h"

CADrawableClef::CADrawableClef(CAClef *musElement, CADrawableStaff *drawableStaff, int x, int y)
 : CADrawableMusElement(musElement, drawableStaff, x, y) {
	_drawableMusElement = CADrawableMusElement::DrawableClef;
	
	switch (clef()->clefType()) {
		case CAClef::Treble:
			_width = 21;
			_height = 68;
			_yPos = y - 15;
			break;
		case CAClef::Bass:
			_width = 22;
			_height = 26;
			_yPos = y;
			break;
		case CAClef::Alto:
		case CAClef::Tenor:
		case CAClef::Soprano:
			_width = 23;
			_height = 34;
			break;
	}
	
	switch (clef()->clefType()) {
		case CAClef::Soprano:
			_yPos = y + 20;
			break;
		case CAClef::Tenor:
			_yPos = y - 8;
			break;
		case CAClef::Alto:
			_yPos = y + 2;
			break;		
	}

	_neededWidth = _width;
	_neededHeight = _height;
}

void CADrawableClef::draw(QPainter *p, CADrawSettings s) {
	QFont font("Emmentaler");
	font.setPixelSize((int)(35*s.z));
	p->setPen(QPen(s.color));
	p->setFont(font);
	
	/*
	 * Clef emmentaler glyphs: The first one is a normal clef (at the beginning of the system) and the other one is a smaller clef (at the center of the system)
	 * - 0xE191: C-clef
	 * - 0xE192: C-clef small
	 * - 0xE193: F-clef
	 * - 0xE194: F-clef small
	 * - 0xE195: G-clef
	 * - 0xE196: G-clef small
	 */
	
	switch (clef()->clefType()) {
		case CAClef::Treble:
			p->drawText(s.x, (int)(s.y + 0.63*height()*s.z), QString(0xE195));
			break;
		case CAClef::Bass:
			p->drawText(s.x, (int)(s.y + 0.32*height()*s.z), QString(0xE193));
			break;
		case CAClef::Alto:
		case CAClef::Soprano:
		case CAClef::Tenor:
			p->drawText(s.x, (int)(s.y + 0.5*height()*s.z), QString(0xE191));
			break;
	}
}

CADrawableClef* CADrawableClef::clone() {
	return (new CADrawableClef(clef(), (CADrawableStaff*)drawableContext(), xPos(), _drawableContext->yPos()));
}
