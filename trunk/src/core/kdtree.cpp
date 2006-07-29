/** @file kdtree.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "drawable/drawable.h"
#include "drawable/drawablemuselement.h"
#include "core/kdtree.h"

CAKDTree::CAKDTree() {
	_maxX = 0;
	_maxY = 0;
}

void CAKDTree::addElement(CADrawable *elt) {
	_list << elt;
	
	if (elt->xPos() + elt->width() > _maxX)
		_maxX = elt->xPos() + elt->width();
	if (elt->yPos() + elt->height() > _maxY)
		_maxY = elt->yPos() + elt->height();		
}

bool CAKDTree::removeElement(CADrawable *elt) {
	delete elt;

	return _list.removeAll(elt);
}

CADrawable* CAKDTree::removeElement(int x, int y, bool autoDelete) {
	CADrawable *elt;
	for (int i=0; i<_list.size(); i++) {
		if (_list.at(i)->bBox().contains(x,y)) {
			elt = _list[i];
			_list.removeAt(i);
			if (autoDelete) delete elt;
			
			calculateMaxXY();
			
			return elt;
		}
	}
	
	return 0;
}

void CAKDTree::clear(bool autoDelete) {
	if (autoDelete) {
		for (int i=0; i<_list.size(); i++)
			delete _list[i];
	}
	
	_list.clear();
}

QList<CADrawable *>* CAKDTree::findInRange(int x, int y, int w, int h) {
	QList<CADrawable *> *l = new QList<CADrawable *>();

	for (int i=0; i<_list.size(); i++) {
		if ( ((_list[i]->xPos() <= x+w) &&						//The object is normal and fits into the area
		     (_list[i]->yPos() <= y+h) &&
		     (_list[i]->xPos() + _list[i]->width() >= x) &&
		     (_list[i]->yPos() + _list[i]->height() >= y)) ||
		     ((_list[i]->width() == 0) &&						//The object is unlimited in width (eg. contexts)
		     (_list[i]->yPos() <= y+h) &&
		     (_list[i]->yPos() + _list[i]->height() >= y)) ||
		     ((_list[i]->height() == 0) &&						//The object is unlimited in height (eg. helper lines)
		     (_list[i]->xPos() <= x+h) &&
		     (_list[i]->xPos() + _list[i]->width() >= x))
		    )
			*l << _list[i];
	}

	return l;	
}

QList<CADrawable *>* CAKDTree::findInRange(QRect *rect) {
	return findInRange(rect->x(), rect->y(), rect->width(), rect->height());
}

CADrawable* CAKDTree::findNearestLeft(int x, bool timeBased, CADrawableContext *context) {
	if (_list.isEmpty())
		return 0;
		
	CADrawable *elt=0;
	QList<CADrawable *>::const_iterator i;
	if (!timeBased) {
		for (i = _list.constBegin(); i != _list.constEnd(); i++) {
			if ( ((!elt) || (((*i)->xPos() + (*i)->width()) > (elt->xPos()+elt->width()))) &&
			     (((*i)->xPos() + (*i)->width()) < x) &&
			     ((!context) || (((CADrawableMusElement*)(*i))->drawableContext() == context))
			   ) {
				elt = *i;
			}
		}
	} else {
		for (i = _list.constBegin(); i != _list.constEnd(); i++) {
			if ( ((!elt) || ((*i)->xPosOrig() > elt->xPosOrig())) &&
			     ((*i)->xPosOrig() < x) &&
			     ((!context) || (((CADrawableMusElement*)(*i))->drawableContext() == context))			     
			     ) {
				elt = *i;
			}
		}
	}
	
	return elt;
}

CADrawable* CAKDTree::findNearestRight(int x, bool timeBased, CADrawableContext *context) {
	if (_list.isEmpty())
		return 0;
		
	CADrawable *elt=0;
	QList<CADrawable *>::const_iterator i;
	if (!timeBased) {
		for (i = _list.constBegin(); i != _list.constEnd(); i++) {
			if ( ((!elt) || ((*i)->xPos() < elt->xPos())) &&
			     ((*i)->xPos() > x) &&
			     ((!context) || (((CADrawableMusElement*)(*i))->drawableContext() == context))			     
			   ) {
				elt = *i;
			}
		}
	} else {
		for (i = _list.constBegin(); i != _list.constEnd(); i++) {
			if ( ((!elt) || ((*i)->xPosOrig() < elt->xPosOrig())) &&
			     ((*i)->xPosOrig() > x) &&
			     ((!context) || (((CADrawableMusElement*)(*i))->drawableContext() == context))			     
			   ) {
				elt = *i;
			}
		}
	}
	
	
	return elt;
}

CADrawable* CAKDTree::findNearestUp(int y) {
	if (_list.isEmpty())
		return 0;
		
	CADrawable *elt=0;
	QList<CADrawable *>::const_iterator i;
	for (i = _list.constBegin(); i != _list.constEnd(); i++) {
		if ( ((!elt) || (((*i)->yPos() + (*i)->height()) > (elt->yPos() + elt->height()))) && (((*i)->yPos()+ (*i)->height()) < y) ) {
			elt = *i;
		}
	}
	return elt;
	
}

CADrawable* CAKDTree::findNearestDown(int y) {
	if (_list.isEmpty())
		return 0;
		
	CADrawable *elt=0;
	QList<CADrawable *>::const_iterator i;
	for (i = _list.constBegin(); i != _list.constEnd(); i++) {
		if ( ((!elt) || ((*i)->yPos() < elt->yPos())) && ((*i)->yPos() > y) ) {
			elt = *i;
		}
	}
	
	return elt;
}

int CAKDTree::getMaxX() {
	return _maxX;
}

int CAKDTree::getMaxY() {
	return _maxY;
}

void CAKDTree::calculateMaxXY() {
	_maxX = 0;
	_maxY = 0;
	for (int i=0; i<_list.size(); i++) {
		if (_list[i]->xPos() + _list[i]->width() > _maxX)
			_maxX = _list[i]->xPos() + _list[i]->width();
		if (_list[i]->yPos() + _list[i]->height() > _maxY)
			_maxY = _list[i]->yPos() + _list[i]->height();
	}
}

void CAKDTree::import(CAKDTree *tree) {
	for (int i=0; i<tree->list().size(); i++)
		_list += tree->list().at(i)->clone();
	
	calculateMaxXY();
}
