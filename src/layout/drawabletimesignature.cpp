/*!
	Copyright (c) 2006-2010, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QString>
#include <QFont>
#include <QPainter>
#include <iostream>
#include "layout/drawabletimesignature.h"
#include "layout/drawablestaff.h"

#include "score/timesignature.h"

CADrawableTimeSignature::CADrawableTimeSignature(CATimeSignature *timeSig, CADrawableStaff *drawableStaff)
 : CADrawableMusElement(timeSig, drawableStaff, DrawableTimeSignature) {
 	QFont font("Emmentaler");
	font.setPixelSize(37);

	// Time signature emmentaler numbers glyphs:
	// - timesig.C44: C 4/4 classical key signature - y0 is the center of the glyph
	// - timesig.C22: C| 2/2 classical key signature - y0 is the center of the glyph
	// - 0..9: y0 is the bottom of the glyph
	//
	switch (timeSignature()->timeSignatureType()) {
		case CATimeSignature::Classical: {	//draw C or C| only, otherwise don't berak, go to Number then
			QGraphicsSimpleTextItem *numberItem = new QGraphicsSimpleTextItem(this);
			numberItem->setFont(font);
			if ((timeSignature()->beat() == 4) && (timeSignature()->beats() == 4)) {
				numberItem->setText( QString(CACanorus::fetaCodepoint("timesig.C44")) );
			} else if ((timeSignature()->beat() == 2) && (timeSignature()->beats() == 2)) {
				numberItem->setText( QString(CACanorus::fetaCodepoint("timesig.C22")) );
			}
			addToGroup( numberItem );
			numberItem->setPos(0, drawableStaff->boundingRect().height()/2 - 51);
			break;
		}
		case CATimeSignature::Number: {
			//write the numbers one by one, first, the number of beats
			QString curBeats = QString::number(timeSignature()->beats());
			QString curBeat = QString::number(timeSignature()->beat());

			QGraphicsSimpleTextItem *numberItem = new QGraphicsSimpleTextItem(this);
			numberItem->setText( curBeats );
			numberItem->setPos( 0, drawableStaff->boundingRect().height()/2 - 66 );
			numberItem->setFont(font);
			addToGroup( numberItem );

			numberItem = new QGraphicsSimpleTextItem(this);
			numberItem->setText( curBeat );
			numberItem->setPos( 0, drawableStaff->boundingRect().height()/2 - 36 );
			numberItem->setFont(font);
			addToGroup( numberItem );

			break;
		}
	}
}

CADrawableTimeSignature::~CADrawableTimeSignature() {
}

/*void CADrawableTimeSignature::draw(QPainter *p, CADrawSettings s) {
	QFont font("Emmentaler");
	font.setPixelSize(qRound(37*s.z));
	p->setPen(QPen(s.color));
	p->setFont(font);

	// Time signature emmentaler numbers glyphs:
	// - timesig.C44: C 4/4 classical key signature - y0 is the center of the glyph
	// - timesig.C22: C| 2/2 classical key signature - y0 is the center of the glyph
	// - 0..9: y0 is the bottom of the glyph
	//
	switch (timeSignature()->timeSignatureType()) {
		case CATimeSignature::Classical: {	//draw C or C| only, otherwise don't berak, go to Number then
			if ((timeSignature()->beat() == 4) && (timeSignature()->beats() == 4)) {
				p->drawText(s.x, qRound(s.y + 0.5*height()*s.z), QString(CACanorus::fetaCodepoint("timesig.C44")));
				break;
			} else if ((timeSignature()->beat() == 2) && (timeSignature()->beats() == 2)) {
				p->drawText(s.x, qRound(s.y + 0.5*height()*s.z), QString(CACanorus::fetaCodepoint("timesig.C22")));
				break;
			}
		}
		case CATimeSignature::Number: {
			//write the numbers one by one, first, the number of beats
			QString curBeats = QString::number(timeSignature()->beats());
			QString curBeat = QString::number(timeSignature()->beat());
			double curX = s.x;
			while (!curBeats.isEmpty() || !curBeat.isEmpty()) {
				if (!curBeats.isEmpty())
					p->drawText(qRound(curX), qRound(s.y + 0.5*drawableContext()->height()*s.z), QString(curBeats[0]));
				if (!curBeat.isEmpty())
					p->drawText(qRound(curX), qRound(s.y + drawableContext()->height()*s.z), QString(curBeat[0]));

				curX += (14*s.z);
				curBeats = curBeats.mid(1);	//trim-off the left-most character
				curBeat = curBeat.mid(1);	//trim-off the left-most character
			}

			break;
		}
	}
}*/
