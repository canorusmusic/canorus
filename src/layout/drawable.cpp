/*!
	Copyright (c) 2006-2010, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsLineItem>

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
	QGraphicsItemGroup::mousePressEvent(event);
}

void CADrawable::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w) {
	QStyleOptionGraphicsItem *_o = const_cast<QStyleOptionGraphicsItem*> (o);
	_o->state &= ~QStyle::State_Selected;
	QGraphicsItemGroup::paint(p, _o, w);
}

/*!
	Reimplement this method and itemChange() to paint the selected elements differently.
	By default it paints all child items to the given color.
 */
void CADrawable::setColor( QColor color ) {
	for (int i=0; i<childItems().size(); i++) {
		if (dynamic_cast<QGraphicsSimpleTextItem*>(childItems()[i])) {
			static_cast<QGraphicsSimpleTextItem*>(childItems()[i])->setBrush( QBrush(color) );
		} else
		if (dynamic_cast<QGraphicsLineItem*>(childItems()[i])) {
			static_cast<QGraphicsLineItem*>(childItems()[i])->setPen( QPen(color) );
		}
	}
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
