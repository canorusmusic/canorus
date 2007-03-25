/*!
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#include "drawable/drawableslur.h"
#include <QPainter>

CADrawableSlur::CADrawableSlur( CASlur *slur, CADrawableContext *c, int x1, int y1, int xMid, int yMid, int x2, int y2  )
 : CADrawableMusElement( slur, c, x1, 0) {
	setWidth( x2 - x1 );
	setYPos( min( y1, y2, yMid ) );
	setHeight( max( y1, y2, yMid ) - yPos() );
	
	setY1( y1 );
	setXMid( xMid );
	setYMid( yMid );
	setY2( y2 );
	
	setNeededWidth( width() );
	setNeededHeight( height() );
}

CADrawableSlur::~CADrawableSlur() {
}

/*!
	Returns the minimum of all the three integers given.
*/
int CADrawableSlur::min(int x, int y, int z) {
	if ( x <= y && y <= z )
		return x;
	else if ( y <= x && x <= z )
		return y;
	else
		return z;
}

/*!
	Returns the maximum of all the three integers given.
*/
int CADrawableSlur::max(int x, int y, int z) {
	if ( x >= y && y >= z )
		return x;
	else if ( y >= x && x >= z )
		return y;
	else
		return z;
}

void CADrawableSlur::draw(QPainter *p, const CADrawSettings s) {
	p->setPen(QPen(s.color));
	
	int minY = min(y1(), yMid(), y2());
	int yLeft = (int)(s.y + (y1()-minY)     * s.z);
	int yMidl = (int)(s.y + (yMid()-minY)   * s.z);
	int xMidl = (int)(s.x + (xMid()-xPos()) * s.z);
	int yRight = (int)(s.y + (y2()-minY)    * s.z);
	
	QPoint points[] = {
		QPoint(s.x, yLeft),
		QPoint(xMidl, yMidl),
		QPoint((int)(s.x + width()*s.z), yRight)
	};
	
	p->drawPolyline(points, 3);
}

CADrawableSlur *CADrawableSlur::clone(CADrawableContext* newContext) {
	return new CADrawableSlur( slur(), newContext?newContext:drawableContext(), xPos(), y1(), xMid(), yMid(), xPos()+width(), y2() );
}
