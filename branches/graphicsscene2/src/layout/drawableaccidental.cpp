/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QFont>
#include <QPainter>

#include "layout/drawableaccidental.h"
#include "score/muselement.h"
#include "layout/drawablecontext.h"
#include "layout/drawableclef.h"
#include "canorus.h"

/*!
	Default constructor.

	\param accs Type of the accidental: 0 - natural, -1 - flat, +1 - sharp, -2 doubleflat, +2 - cross etc.
	\param musElement Pointer to the according musElement which the accidental represents (usually a CANote or CAKeySignature).
	\param drawableContext Pointer to the according drawable context which the accidental belongs to (usually CADrawableStaff or CADrawableFiguredBass).
	\param x Left X-coordinate of the accidental.
	\param y Center Y-coordinate of the accidental.
*/
CADrawableAccidental::CADrawableAccidental(signed char accs, CAMusElement *musElement, CADrawableContext *drawableContext, double x, double y)
 : CADrawableMusElement(musElement, drawableContext, x, y) {
 	setDrawableMusElementType( DrawableAccidental );
 	setSelectable( false );

 	setWidth( 8 );
 	setHeight( 14 );
 	_accs = accs;

 	if (accs==0) {
 		setYPos( y - height()/2 );
 	} else if (accs==1) {
 		setYPos( y - height()/2 );
 	} else if (accs==-1) {
  		setYPos( y - height()/2 - 5 );
 	} else if (accs==2) {
 		setHeight( 6 );
  		setYPos( y - height()/2 );
 	} else if (accs==-2) {
  		setYPos( y - height()/2 - 5 );
 		setXPos( x );
 		setWidth( 12 );
 	}

 	_centerX = x;
 	_centerY = y;
}

CADrawableAccidental::~CADrawableAccidental() {
}

void CADrawableAccidental::draw(QPainter *p, CADrawSettings s) {
	QFont font("Emmentaler");
	font.setPixelSize(qRound(34*s.z));
	p->setPen(QPen(s.color));
	p->setFont(font);

	switch (_accs) {
		case 0:
			p->drawText(s.x, s.y + qRound(height()/2*s.z), QString(CACanorus::fetaCodepoint("accidentals.natural")));
			break;
		case 1:
			p->drawText(s.x, s.y + qRound((height()/2 + 0.3)*s.z), QString(CACanorus::fetaCodepoint("accidentals.sharp")));
			break;
		case -1:
			p->drawText(s.x, s.y + qRound((height()/2 + 5)*s.z), QString(CACanorus::fetaCodepoint("accidentals.flat")));
			break;
		case 2:
			p->drawText(s.x, s.y + qRound(height()/2*s.z), QString(CACanorus::fetaCodepoint("accidentals.doublesharp")));
			break;
		case -2:
			p->drawText(s.x, s.y + qRound((height()/2 + 5)*s.z), QString(CACanorus::fetaCodepoint("accidentals.flatflat")));
			break;
	}
}

CADrawableAccidental *CADrawableAccidental::clone(CADrawableContext* newContext) {
	return new CADrawableAccidental(_accs, _musElement, (newContext)?newContext:_drawableContext, _centerX, _centerY);
}
