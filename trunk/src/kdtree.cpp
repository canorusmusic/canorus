/** @file kdtree.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "drawable.h"
#include "kdtree.h"

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

bool CAKDTree::removeElement(int x, int y) {
	for (int i=0; i<_list.size(); i++) {
		if (_list.at(i)->bBox().contains(x,y)) {
			_list.removeAt(i);
			calculateMaxXY();
			return true;
		}
	}
	
	return false;
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

CADrawable* CAKDTree::findNearestLeft(int x, bool timeBased) {
	if (_list.isEmpty())
		return 0;
		
	CADrawable *elt;
	CADrawable *cur;
	if (!timeBased) {
		for (int i=0; i<_list.size(); ++i, cur=_list[i])
			if ( ((!elt) || ((cur->xPos()+cur->width()) > (elt->xPos()+elt->width()))) && ((cur->xPos()+cur->width()) < x) )
				elt = cur;
	} else {
		for (int i=0; i<_list.size(); ++i, cur=_list[i])
			if ( ((!elt) || (cur->xPosOrig() > elt->xPosOrig())) && (cur->xPosOrig() < x) )
				elt = cur;		
	}
	
	return elt;
}

CADrawable* CAKDTree::findNearestRight(int x, bool timeBased) {
	if (_list.isEmpty())
		return 0;
		
	CADrawable *elt;
	if (!timeBased) {
		for (int i=0; i<_list.size(); i++)
			if ( ((!elt) || (_list[i]->xPos() < elt->xPos())) && (_list[i]->xPos() > x) )
				elt = _list[i];
	} else {
		for (int i=0; i<_list.size(); i++)
			if ( ((!elt) || (_list[i]->xPosOrig() < elt->xPosOrig())) && (_list[i]->xPosOrig() > x) )
				elt = _list[i];
	}
	
	
	return elt;
}

CADrawable* CAKDTree::findNearestUp(int y) {
	if (_list.isEmpty())
		return 0;
		
	CADrawable *elt;
	CADrawable *cur;
	for (int i=0; i<_list.size(); i++, cur=_list[i])
		if ( ((!elt) || ((cur->yPos()+cur->height()) > (elt->yPos()+elt->height()))) && ((cur->yPos()+cur->height()) < y) )
			elt = _list[i];
	
	return elt;
	
}

CADrawable* CAKDTree::findNearestDown(int y) {
	if (_list.isEmpty())
		return 0;
		
	CADrawable *elt;
	for (int i=0; i<_list.size(); i++)
		if ( ((!elt) || (_list[i]->yPos() < elt->yPos())) && (_list[i]->yPos() > y) )
			elt = _list[i];
	
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
