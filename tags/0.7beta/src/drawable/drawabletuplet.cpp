/*!
 * Copyright (c) 2008, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 *
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#include "drawable/drawabletuplet.h"
#include "drawable/drawablecontext.h"
#include <QPen>
#include <QPainter>
#include <QFont>

CADrawableTuplet::CADrawableTuplet( CATuplet *tuplet, CADrawableContext *c, int x1, int y1, int x2, int y2 )
 : CADrawableMusElement( tuplet, c, x1, 0 ) {
	setDrawableMusElementType( DrawableTuplet );

	setWidth( x2-x1 );
	setHeight( (abs(y2-y1)>5)?abs(y2-y1):8 );
	setYPos( (c && qMin(y1, y2)>c->yPos())?qMin(y1, y2):(qMin(y1,y2)-height()) );

	setNeededWidth( width() );
	setNeededHeight( height() );
}

CADrawableTuplet::~CADrawableTuplet() {
}

void CADrawableTuplet::draw(QPainter *p, const CADrawSettings s) {
	QPen pen(s.color);
	pen.setWidth( qRound(1.2*s.z) );
	pen.setCapStyle( Qt::RoundCap );
	p->setPen( pen );

	int minY = qMin(yPos(), yPos())-8;
	int yLeft = qRound(s.y + (yPos()-minY)       * s.z);
	int yMidl = qRound(s.y + (yPos()-minY)       * s.z);
	int xMidl = qRound(s.x + (width()/2.0) * s.z);
	int yRight = qRound(s.y + (yPos()-minY)      * s.z);
	int deltaY1 = (yMidl - yLeft);
	int deltaY2 = (yRight - yMidl);
	int deltaX1 = xMidl - s.x;
	int deltaX2 = qRound(s.x + width()*s.z - xMidl);

	// generate an array of points for the rounded slur using the exponent shape
	QPoint points[9];
	points[0] = QPoint( s.x, yLeft );
	points[1] = QPoint( qRound(s.x + 0.1*deltaX1), qRound(yLeft + deltaY1*0.34) );
	points[2] = QPoint( qRound(s.x + 0.2*deltaX1), qRound(yLeft + deltaY1*0.53) );
	points[3] = QPoint( qRound(s.x + 0.3*deltaX1), qRound(yLeft + deltaY1*0.71) );
	points[4] = QPoint( qRound(s.x + 0.4*deltaX1), qRound(yLeft + deltaY1*0.79) );
	points[5] = QPoint( qRound(s.x + 0.5*deltaX1), qRound(yLeft + deltaY1*0.86) );
	points[6] = QPoint( qRound(s.x + 0.6*deltaX1), qRound(yLeft + deltaY1*0.90) );
	points[7] = QPoint( qRound(s.x + 0.7*deltaX1), qRound(yLeft + deltaY1*0.94) );
	points[8] = QPoint( qRound(s.x + 0.8*deltaX1), qRound(yLeft + deltaY1*0.95) );
	p->drawPolyline(points, 9);

	points[0] = QPoint( qRound(xMidl + 0.2*deltaX2), qRound(yMidl + deltaY2*0.05) );
	points[1] = QPoint( qRound(xMidl + 0.3*deltaX2), qRound(yMidl + deltaY2*0.06) );
	points[2] = QPoint( qRound(xMidl + 0.4*deltaX2), qRound(yMidl + deltaY2*0.10) );
	points[3] = QPoint( qRound(xMidl + 0.5*deltaX2), qRound(yMidl + deltaY2*0.14) );
	points[4] = QPoint( qRound(xMidl + 0.6*deltaX2), qRound(yMidl + deltaY2*0.21) );
	points[5] = QPoint( qRound(xMidl + 0.7*deltaX2), qRound(yMidl + deltaY2*0.29) );
	points[6] = QPoint( qRound(xMidl + 0.8*deltaX2), qRound(yMidl + deltaY2*0.47) );
	points[7] = QPoint( qRound(xMidl + 0.9*deltaX2), qRound(yMidl + deltaY2*0.66) );
	points[8] = QPoint( qRound(s.x+width()*s.z), yRight );
	p->drawPolyline(points, 9);

	QFont font("Emmentaler");
	font.setPixelSize( qRound(16*1.3*s.z) );
	font.setItalic( true );
	p->setFont(font);
	p->drawText( s.x + qRound((width()/2.0-3)*s.z), s.y + qRound((height()/2.0+9)*s.z), QString::number( tuplet()->number() ) );
}

CADrawableTuplet *CADrawableTuplet::clone(CADrawableContext* newContext) {
	return new CADrawableTuplet( tuplet(), newContext?newContext:drawableContext(), x1(), y1(), x2(), y2() );
}
