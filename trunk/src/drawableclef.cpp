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

#include "drawableclef.h"
#include "drawablecontext.h"

#include "clef.h"

CADrawableClef::CADrawableClef(CAClef *musElement, CADrawableContext *drawableContext, int x, int y)
 : CADrawableMusElement(musElement, drawableContext, x, y) {
	_drawableMusElement = CADrawableMusElement::DrawableClef;
	
	switch (clef()->clefType()) {
		case CAClef::Treble:
			_width = 21;
			_height = 68;
			_yPos = y - 15;
	}

	_neededWidth = _width;
	_neededHeight = _height;
}

void CADrawableClef::draw(QPainter *p, CADrawSettings s) {
	QFont font("Emmentaler");
	font.setPixelSize((int)(35*s.z));
	p->setPen(QPen(s.color));
	p->setFont(font);
	
	switch (clef()->clefType()) {
		case CAClef::Treble:
			p->drawText(s.x, (int)(s.y + 0.63*height()*s.z), QString(0xE195));
			
			break;
	}
}

CADrawableClef* CADrawableClef::clone() {
	return (new CADrawableClef(clef(), drawableContext(), xPos(), yPos()+15));
}
