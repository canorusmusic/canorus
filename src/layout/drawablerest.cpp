/*!
	Copyright (c) 2006-2010, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "layout/drawablerest.h"
#include "layout/drawablecontext.h"
#include "layout/drawablestaff.h"
#include "score/rest.h"
#include "canorus.h"

CADrawableRest::CADrawableRest(CARest *r, CADrawableContext *drawableContext)
 : CADrawableMusElement(r, drawableContext, DrawableRest) {
	QFont font("Emmentaler");
	font.setPixelSize(35);

	// Draw rest
	QGraphicsSimpleTextItem *item = 0;
	switch ( rest()->playableLength().musicLength() ) {
	case CAPlayableLength::HundredTwentyEighth: {
		item = new QGraphicsSimpleTextItem(QString(CACanorus::fetaCodepoint("rests.7")), this);
		item->setFont(font);
		addToGroup(item);
		item->setPos(4, 2.6*static_cast<CADrawableStaff*>(_drawableContext)->lineSpace());
		break;
	}
	case CAPlayableLength::SixtyFourth: {
		item = new QGraphicsSimpleTextItem(QString(CACanorus::fetaCodepoint("rests.6")), this);
		item->setFont(font);
		addToGroup(item);
		item->setPos(3, 1.75*static_cast<CADrawableStaff*>(_drawableContext)->lineSpace());
		break;
	}
	case CAPlayableLength::ThirtySecond: {
		item = new QGraphicsSimpleTextItem(QString(CACanorus::fetaCodepoint("rests.5")), this);
		item->setFont(font);
		addToGroup(item);
		item->setPos(2.5, 1.8*static_cast<CADrawableStaff*>(_drawableContext)->lineSpace());
		break;
	}
	case CAPlayableLength::Sixteenth: {
		item = new QGraphicsSimpleTextItem(QString(CACanorus::fetaCodepoint("rests.4")), this);
		item->setFont(font);
		addToGroup(item);
		item->setPos(1.0, static_cast<CADrawableStaff*>(_drawableContext)->lineSpace()-0.9);
		break;
	}
	case CAPlayableLength::Eighth: {
		item = new QGraphicsSimpleTextItem(QString(CACanorus::fetaCodepoint("rests.3")), this);
		item->setFont(font);
		addToGroup(item);
		item->setPos(0, static_cast<CADrawableStaff*>(_drawableContext)->lineSpace()-0.9);
		break;
	}
	case CAPlayableLength::Quarter: {
		item = new QGraphicsSimpleTextItem(QString(CACanorus::fetaCodepoint("rests.2")), this);
		item->setFont(font);
		addToGroup(item);
		item->setPos(0, 0.5*item->boundingRect().height());
		break;
	}
	case CAPlayableLength::Half: {
		item = new QGraphicsSimpleTextItem(QString(CACanorus::fetaCodepoint("rests.1")), this);
		item->setFont(font);
		addToGroup(item);
		item->setPos(0, item->boundingRect().height() + 0.5);
		break;
	}
	case CAPlayableLength::Whole: {
		item = new QGraphicsSimpleTextItem(QString(CACanorus::fetaCodepoint("rests.0")), this);
		item->setFont(font);
		addToGroup(item);
		break;
	}
	case CAPlayableLength::Breve: {
		item = new QGraphicsSimpleTextItem(QString(CACanorus::fetaCodepoint("rests.M1")), this);
		item->setFont(font);
		addToGroup(item);
		item->setPos(0, item->boundingRect().height());
		break;
	}
	}

	if (item) {
		item->moveBy(0, -47);
	}

	// Draw dots
	double dotOffset = 3;
	double dotWidth  = 2.5;
	QBrush dotBrush(Qt::SolidPattern);
	for (int i=0; i<rest()->playableLength().dotted(); i++) {
		QGraphicsEllipseItem *item = new QGraphicsEllipseItem(boundingRect().width() + dotOffset, -2.5, dotWidth, dotWidth, this);
		item->setBrush(dotBrush);
		addToGroup(item);
	}
}

CADrawableRest::~CADrawableRest() {
}

/*void CADrawableRest::draw(QPainter *p, CADrawSettings s) {
	QFont font("Emmentaler");
	font.setPixelSize(qRound(35*s.z));

	p->setPen(QPen(s.color));
	p->setFont(font);

	QPen pen;
	switch ( rest()->playableLength().musicLength() ) {
	case CAPlayableLength::HundredTwentyEighth: {
		p->drawText(qRound(s.x + 4*s.z), qRound(s.y + (2.6*((CADrawableStaff*)_drawableContext)->lineSpace())*s.z), QString(CACanorus::fetaCodepoint("rests.7")));
		break;
	}
	case CAPlayableLength::SixtyFourth: {
		p->drawText(qRound(s.x + 3*s.z), qRound(s.y + (1.75*((CADrawableStaff*)_drawableContext)->lineSpace())*s.z), QString(CACanorus::fetaCodepoint("rests.6")));
		break;
	}
	case CAPlayableLength::ThirtySecond: {
		p->drawText(qRound(s.x + 2.5*s.z), qRound(s.y + (1.8*((CADrawableStaff*)_drawableContext)->lineSpace())*s.z), QString(CACanorus::fetaCodepoint("rests.5")));
		break;
	}
	case CAPlayableLength::Sixteenth: {
		p->drawText(qRound(s.x + 1*s.z), qRound(s.y + (((CADrawableStaff*)_drawableContext)->lineSpace()-0.9)*s.z), QString(CACanorus::fetaCodepoint("rests.4")));
		break;
	}
	case CAPlayableLength::Eighth: {
		p->drawText(s.x, qRound(s.y + (((CADrawableStaff*)_drawableContext)->lineSpace()-0.9)*s.z), QString(CACanorus::fetaCodepoint("rests.3")));
		break;
	}
	case CAPlayableLength::Quarter: {
		p->drawText(s.x,qRound(s.y + 0.5*height()*s.z),QString(CACanorus::fetaCodepoint("rests.2")));
		break;
	}
	case CAPlayableLength::Half: {
		p->drawText(s.x,qRound(s.y + height()*s.z + 0.5), QString(CACanorus::fetaCodepoint("rests.1")));
		break;
	}
	case CAPlayableLength::Whole: {
		p->drawText(s.x, s.y, QString(CACanorus::fetaCodepoint("rests.0")));
		break;
	}
	case CAPlayableLength::Breve: {
		p->drawText(s.x, qRound(s.y + height()*s.z), QString(CACanorus::fetaCodepoint("rests.M1")));
		break;
	}
	}

	///////////////
	// Draw Dots //
	///////////////
	float delta=4*s.z;
	for (int i=0; i<rest()->playableLength().dotted(); i++) {
		pen.setWidth(qRound(2.7*s.z+0.5) + 1);
		pen.setCapStyle(Qt::RoundCap);
		pen.setColor(s.color);
		p->setPen(pen);
		p->drawPoint(qRound(s.x + _restWidth*s.z + delta), qRound(s.y + 0.3*_height*s.z));
		delta += 3*s.z;
	}

}*/
