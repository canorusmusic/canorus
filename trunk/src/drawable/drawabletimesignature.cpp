/** @file drawabletimesignature.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QString>
#include <QFont>
#include <QPainter>

#include "drawable/drawabletimesignature.h"
#include "drawable/drawablestaff.h"

#include "core/timesignature.h"

CADrawableTimeSignature::CADrawableTimeSignature(CATimeSignature *timeSig, CADrawableStaff *drawableStaff, int x, int y)
 : CADrawableMusElement(timeSig, drawableStaff, x, y) {
 	_drawableMusElement = CADrawableMusElement::DrawableTimeSignature;
 	
 	//determine the width - number of characters needed to draw the beat or beats (which one is bigger?) times the width of a single character
 	_width = 25 + 27*((timeSignature()->beats()/10 > timeSignature()->beat()/10) ? timeSignature()->beats()/10 : timeSignature()->beat()/10);
 	_height = drawableStaff->height();
 	
 	_neededWidth = _width;
 	_neededHeight = _height;
}

CADrawableTimeSignature::~CADrawableTimeSignature() {
}

void CADrawableTimeSignature::draw(QPainter *p, CADrawSettings s) {
	QFont font("Emmentaler");
	font.setPixelSize((int)(35*s.z));
	p->setPen(QPen(s.color));
	p->setFont(font);
	
	/*
	 * Time signature emmentaler numbers glyphs:
	 * - 0xE19B: C 4/4 classical key signature - y0 is the center of the glyph
	 * - 0xE19C: C| 2/2 classical key signature - y0 is the center of the glyph
	 * - 0x30: 0 - y0 are the bottom of the glyphs
	 * - 0x31: 1
	 * - 0x32: 2
	 * - 0x33: 3
	 * - 0x34: 4
	 * - 0x35: 5
	 * - 0x36: 6
	 * - 0x37: 7
	 * - 0x38: 8
	 * - 0x39: 9
	 */
	switch (timeSignature()->timeSignatureType()) {
		case CATimeSignature::Classical: {	//draw C or C| only, otherwise don't berak, go to Number then
			if ((timeSignature()->beat() == 4) && (timeSignature()->beats() == 4)) {
				p->drawText(s.x, (int)(s.y + 0.5*height()*s.z), QString(0xE19B));	//draw C
				break;
			} else if ((timeSignature()->beat() == 2) && (timeSignature()->beats() == 2)) {
				p->drawText(s.x, (int)(s.y + 0.5*height()*s.z), QString(0xE19C));	//draw C|
				break;
			}
		}
		case CATimeSignature::Number: {
			//write the numbers one by one, first, the number of beats
			QString curBeats = QString::number(timeSignature()->beats());
			int curX = s.x;
			while (!curBeats.isEmpty()) {
				p->drawText(curX, (int)(s.y + 0.5*drawableContext()->height()*s.z), QString(curBeats[0]));
				
				curX+=27;
				curBeats = curBeats.mid(1);	//trim-off the left-most character
			}
			
			//and now the beat
			QString curBeat = QString::number(timeSignature()->beat());
			curX = s.x;
			while (!curBeat.isEmpty()) {
				p->drawText(curX, (int)(s.y + drawableContext()->height()*s.z), QString(curBeat[0]));
				
				curX+=27;
				curBeat = curBeat.mid(1);	//trim-off the left-most character
			}
			
			break;
		}
	}
}

CADrawableTimeSignature* CADrawableTimeSignature::clone() {
	return (new CADrawableTimeSignature(timeSignature(), (CADrawableStaff*)drawableContext(), xPos(), _drawableContext->yPos()));
}