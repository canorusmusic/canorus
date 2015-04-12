/*!
	Copyright (c) 2015, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPen>
#include <QPainter>

#include "layout/drawablenotecheckererror.h"

CADrawableNoteCheckerError::CADrawableNoteCheckerError(CANoteCheckerError *nce, CADrawable *dTarget)
 : CADrawable(dTarget->xPos()-5, dTarget->yPos()+dTarget->height()+5), _noteCheckerError(nce) {
	setWidth( dTarget->width()+10 );
	setHeight( 5 );
}

void CADrawableNoteCheckerError::draw(QPainter *p, const CADrawSettings s) {
	QPen pen;
	pen.setWidth(2);
	pen.setCapStyle(Qt::SquareCap);
	pen.setColor(s.color);
	p->setPen(pen);
	
	// draw wavy line
	int Xstep=2;
	int Ystep=1;
	int oldX=s.x;
	int x=oldX+2;
	int oldY=s.y;
	int y=oldY+Ystep;
	while (oldX<s.x+width()*s.z) {
		p->drawLine(oldX, oldY, x, y);
		oldX=x;
		oldY=y;
		x+=Xstep;
		y+=Ystep;
		Ystep*=(-1);
	}
}

CADrawable *CADrawableNoteCheckerError::clone() {
	return 0; //TODO: Kinda tricky, we require the instance of the new cloned drawable target
}
