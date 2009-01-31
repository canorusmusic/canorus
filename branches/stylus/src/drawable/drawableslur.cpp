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
 	setDrawableMusElementType( DrawableSlur );
 	setX1( x1 );
 	setY1( y1 );
	setXMid( xMid );
	setYMid( yMid );
 	setX2( x2 );
 	setY2( y2 );
 	
 	updateGeometry(); // sets width, height, xPos, yPos
 	
	setNeededWidth( width() );
	setNeededHeight( height() );
}

CADrawableSlur::~CADrawableSlur() {
}

void CADrawableSlur::updateGeometry() {
	setXPos( min(x1(), xMid(), x2()) );
	setWidth( max(x1(), xMid(), x2()) - xPos() );
	
	setYPos( min(y1(), yMid(), y2()) );
	setHeight( max(y1(), yMid(), y2()) - yPos() );
}

/*!
	Returns the minimum of all the three integers given.
*/
int CADrawableSlur::min(int x, int y, int z) {
	if ( x <= y && x <= z )
		return x;
	else if ( y <= x && y <= z )
		return y;
	else
		return z;
}

/*!
	Returns the maximum of all the three integers given.
*/
int CADrawableSlur::max(int x, int y, int z) {
	if ( x >= y && x >= z )
		return x;
	else if ( y >= x && y >= z )
		return y;
	else
		return z;
}

void CADrawableSlur::draw(QPainter *p, const CADrawSettings s) {
	p->setRenderHint(QPainter::Antialiasing);
	QPen pen(s.color);
	pen.setWidth( qRound(1.2*s.z) );
	pen.setCapStyle( Qt::RoundCap );
	switch (slur()->slurStyle()) {
		case CASlur::SlurSolid:
			pen.setStyle( Qt::SolidLine );
			break;
		case CASlur::SlurDotted:
			pen.setStyle( Qt::DotLine );
			break;
	}
	p->setPen( pen );
	
	int minY = min(y1(), yMid(), y2());
	int yLeft = qRound(s.y + (y1()-minY)     * s.z);
	int yMidl = qRound(s.y + (yMid()-minY)   * s.z);
	int xMidl = qRound(s.x + (xMid()-xPos()) * s.z);
	int yRight = qRound(s.y + (y2()-minY)    * s.z);
	int deltaY1 = (yMidl - yLeft);
	int deltaY2 = (yRight - yMidl);
	int deltaX1 = xMidl - s.x;
	int deltaX2 = qRound(s.x + width()*s.z - xMidl);
	
	// generate an array of points for the rounded slur using the exponent shape
	QPoint points[21];
	points[0] = QPoint( s.x, yLeft );
	points[1] = QPoint( qRound(s.x + 0.1*deltaX1), qRound(yLeft + deltaY1*0.34) );
	points[2] = QPoint( qRound(s.x + 0.2*deltaX1), qRound(yLeft + deltaY1*0.53) );
	points[3] = QPoint( qRound(s.x + 0.3*deltaX1), qRound(yLeft + deltaY1*0.71) );
	points[4] = QPoint( qRound(s.x + 0.4*deltaX1), qRound(yLeft + deltaY1*0.79) );
	points[5] = QPoint( qRound(s.x + 0.5*deltaX1), qRound(yLeft + deltaY1*0.86) );
	points[6] = QPoint( qRound(s.x + 0.6*deltaX1), qRound(yLeft + deltaY1*0.90) );
	points[7] = QPoint( qRound(s.x + 0.7*deltaX1), qRound(yLeft + deltaY1*0.94) );
	points[8] = QPoint( qRound(s.x + 0.8*deltaX1), qRound(yLeft + deltaY1*0.95) );
	points[9] = QPoint( qRound(s.x + 0.9*deltaX1), qRound(yLeft + deltaY1*0.97) );
	points[10] = QPoint( xMidl, qRound(yMidl + deltaY2*0.02) );
	points[11] = QPoint( qRound(xMidl + 0.1*deltaX2), qRound(yMidl + deltaY2*0.03) );
	points[12] = QPoint( qRound(xMidl + 0.2*deltaX2), qRound(yMidl + deltaY2*0.05) );
	points[13] = QPoint( qRound(xMidl + 0.3*deltaX2), qRound(yMidl + deltaY2*0.06) );
	points[14] = QPoint( qRound(xMidl + 0.4*deltaX2), qRound(yMidl + deltaY2*0.10) );
	points[15] = QPoint( qRound(xMidl + 0.5*deltaX2), qRound(yMidl + deltaY2*0.14) );
	points[16] = QPoint( qRound(xMidl + 0.6*deltaX2), qRound(yMidl + deltaY2*0.21) );
	points[17] = QPoint( qRound(xMidl + 0.7*deltaX2), qRound(yMidl + deltaY2*0.29) );
	points[18] = QPoint( qRound(xMidl + 0.8*deltaX2), qRound(yMidl + deltaY2*0.47) );
	points[19] = QPoint( qRound(xMidl + 0.9*deltaX2), qRound(yMidl + deltaY2*0.66) );
	points[20] = QPoint( qRound(s.x+width()*s.z), yRight );
	
	p->drawPolyline(points, 21);
}

CADrawableSlur *CADrawableSlur::clone(CADrawableContext* newContext) {
	return new CADrawableSlur( slur(), newContext?newContext:drawableContext(), xPos(), y1(), xMid(), yMid(), xPos()+width(), y2() );
}
