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
#include <iostream>
#include "drawable/drawabletimesignature.h"
#include "drawable/drawablestaff.h"

#include "core/timesignature.h"

CADrawableTimeSignature::CADrawableTimeSignature(CATimeSignature *timeSig, CADrawableStaff *drawableStaff, int x, int y)
 : CADrawableMusElement(timeSig, drawableStaff, x, y) {
 	_drawableMusElementType = CADrawableMusElement::DrawableTimeSignature;
 	
 	if ((timeSignature()->timeSignatureType() == CATimeSignature::Classical) && (timeSignature()->beat() == 4) && (timeSignature()->beats() == 4)) {
 		_width = 16;
 		_height = 20;
 		_yPos = (int)(drawableContext()->yCenter() - 0.5*_height + 0.5);
 	} else if ((timeSignature()->timeSignatureType() == CATimeSignature::Classical) && (timeSignature()->beat() == 2) && (timeSignature()->beats() == 2)) {
 		_width = 16;
 		_height = 24;
 		_yPos = (int)(drawableContext()->yCenter() - 0.5*_height + 0.5);
 	} else { //determine the width - number of characters needed to draw the beat and beats times the width of a single character
 		_width = 14;
 		QString beats = QString::number(timeSignature()->beats()).mid(1);	//cut-off the first character already
 		QString beat = QString::number(timeSignature()->beat()).mid(1);	//cut-off the first character already
		while ((!beats.isEmpty()) || (!beat.isEmpty())) {
 			_width += 15;	//blank + number width
 			
 			beats = beats.mid(1);
 			beat = beat.mid(1);
		}
 		_height = drawableStaff->height();
 	}
 
 	_neededWidth = _width;
 	_neededHeight = _height;
}

CADrawableTimeSignature::~CADrawableTimeSignature() {
}

void CADrawableTimeSignature::draw(QPainter *p, CADrawSettings s) {
	QFont font("Emmentaler");
	font.setPixelSize((int)(37*s.z));
	p->setPen(QPen(s.color));
	p->setFont(font);
	
	/*
	 * Time signature emmentaler numbers glyphs:
	 * - timesig.C44: C 4/4 classical key signature - y0 is the center of the glyph
	 * - timesig.C22: C| 2/2 classical key signature - y0 is the center of the glyph
	 * - 0..9: y0 is the bottom of the glyph
	 */
	switch (timeSignature()->timeSignatureType()) {
		case CATimeSignature::Classical: {	//draw C or C| only, otherwise don't berak, go to Number then
			if ((timeSignature()->beat() == 4) && (timeSignature()->beats() == 4)) {
				p->drawText(s.x, (int)(s.y + 0.5*height()*s.z), QString(CACanorus::fetaCodepoint("timesig.C44")));
				break;
			} else if ((timeSignature()->beat() == 2) && (timeSignature()->beats() == 2)) {
				p->drawText(s.x, (int)(s.y + 0.5*height()*s.z), QString(CACanorus::fetaCodepoint("timesig.C22")));
				break;
			}
		}
		case CATimeSignature::Number: {
			//write the numbers one by one, first, the number of beats
			QString curBeats = QString::number(timeSignature()->beats());
			QString curBeat = QString::number(timeSignature()->beat());
			int curX = s.x;
			while (!curBeats.isEmpty() || !curBeat.isEmpty()) {
				if (!curBeats.isEmpty())				
					p->drawText(curX, (int)(s.y + 0.5*drawableContext()->height()*s.z), QString(curBeats[0]));
				if (!curBeat.isEmpty())
					p->drawText(curX, (int)(s.y + drawableContext()->height()*s.z), QString(curBeat[0]));
				
				curX += (int)(14*s.z + 0.5);
				curBeats = curBeats.mid(1);	//trim-off the left-most character
				curBeat = curBeat.mid(1);	//trim-off the left-most character
			}

			break;
		}
	}
}

CADrawableTimeSignature* CADrawableTimeSignature::clone(CADrawableContext* newContext) {
	return (new CADrawableTimeSignature(timeSignature(), (CADrawableStaff*)((newContext)?newContext:_drawableContext), xPos(), _drawableContext->yPos()));
}
