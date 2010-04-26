/*!
	Copyright (c) 2006-2010, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPainter>

#include "layout/drawable.h"
#include "layout/drawablemuselement.h"
#include "layout/drawablecontext.h"

const int CADrawable::SCALE_HANDLES_SIZE = 2;

CADrawable::CADrawable( const CADrawableType& drawableType )
 : QGraphicsItemGroup(),
   _drawableType( drawableType ),
   _hScalable(false), _vScalable(false) {
	setFlags( QGraphicsItem::ItemIsSelectable );
}

void CADrawable::mousePressEvent( QGraphicsSceneMouseEvent * event ) {
	setColor( Qt::red );
	QGraphicsItemGroup::mousePressEvent(event);
}

/*void CADrawable::drawHScaleHandles( QPainter *p, CADrawSettings s ) {
	p->setPen(QPen(s.color));
	p->drawRect( s.x - qRound((SCALE_HANDLES_SIZE*s.z)/2), s.y + qRound((height()*s.z)/2 - (SCALE_HANDLES_SIZE*s.z)/2),
			     qRound(SCALE_HANDLES_SIZE*s.z), qRound(SCALE_HANDLES_SIZE*s.z) );
	p->drawRect( s.x + qRound(((width() - SCALE_HANDLES_SIZE/2.0)*s.z)), s.y + qRound((height()/2.0 - SCALE_HANDLES_SIZE/2.0)*s.z),
			     qRound(SCALE_HANDLES_SIZE*s.z), qRound(SCALE_HANDLES_SIZE*s.z) );
}

void CADrawable::drawVScaleHandles( QPainter *p, CADrawSettings s ) {
	p->setPen(QPen(s.color));
	p->drawRect( s.x + qRound((width()*s.z)/2 - (SCALE_HANDLES_SIZE*s.z)/2), s.y - qRound((SCALE_HANDLES_SIZE*s.z)/2),
			     qRound(SCALE_HANDLES_SIZE*s.z), qRound(SCALE_HANDLES_SIZE*s.z) );
	p->drawRect( s.x + qRound((width()*s.z)/2 - (SCALE_HANDLES_SIZE*s.z)/2), s.y + qRound(((height() - SCALE_HANDLES_SIZE/2.0)*s.z)),
			     qRound(SCALE_HANDLES_SIZE*s.z), qRound(SCALE_HANDLES_SIZE*s.z) );
}
*/
