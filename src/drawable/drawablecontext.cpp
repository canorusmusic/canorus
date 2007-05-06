/*! 
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "drawable/drawablecontext.h"

CADrawableContext::CADrawableContext(CAContext *c, int x, int y) : CADrawable(x, y) {
	_drawableType = CADrawable::DrawableContext;
	_context = c;
}

/*!
	Returns a list of drawable music elements the current drawable context includes between
	the horizontal coordinates \a x1 and \a x2.
	The element is in a list already if only part of the element is touched by the region.
	That is the first returned element's left border is smaller than \a x1 and the last returned element's
	right border is larger than \a x2.
*/
QList<CADrawableMusElement*> CADrawableContext::findInRange( int x1, int x2 ) {
	int i;
	QList<CADrawableMusElement*> list;
	for (i=0;
	     i<_drawableMusElementList.size() &&
	      x1 > (_drawableMusElementList[i]->xPos() + _drawableMusElementList[i]->width());
	     i++);
	
	if (i==_drawableMusElementList.size()) return list;
	
	for (;
	     i<_drawableMusElementList.size() && x2 > _drawableMusElementList[i]->xPos();
	     list << _drawableMusElementList[i], i++);
	
	return list;
}
