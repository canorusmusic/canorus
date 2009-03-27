/*!
	Copyright (c) 2006-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPainter>

#include "drawable/drawable.h"
#include "drawable/drawablemuselement.h"
#include "drawable/drawablecontext.h"

const int CADrawable::SCALE_HANDLES_SIZE = 2;

CADrawable::CADrawable(double x, double y)
 : _xPos(x), _yPos(y),
   _neededSpaceWidth(0), _neededSpaceHeight(0),
   _visible(true), _selectable(true),
   _hScalable(false), _vScalable(false) {
}

CADrawable* CADrawable::clone() {
	// We only reach CADrawable::clone() if this is a CADrawableMusElement, otherwise CADrawableContext::clone() will be called (this is a non-pure virtual function).
	return static_cast<CADrawableMusElement*>(this)->clone();
}

void CADrawable::drawHScaleHandles( QPainter *p, CADrawSettings s ) {
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
