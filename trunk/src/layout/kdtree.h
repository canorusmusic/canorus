/*!
	Copyright (c) 2006-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef KDTREE_H
#define KDTREE_H

#include <QMultiMap>
#include <QRect>
#include <limits> // max double for managing staffs with unlimited width
#include <iostream> // debugging

#include "layout/drawable.h"
#include "layout/drawablecontext.h"
#include "layout/drawablemuselement.h"

#include "score/muselement.h"
#include "score/playable.h"
#include "score/voice.h"

/*!
	\class CAKDTree
	\brief Space partitioning structure for fast access to drawable elements on canvas

	This class is a data structure focused on efficient access to the drawable
	instances of the music elements. It is based on an interval tree and employs
	multiple QMap instances to enable efficient query of elements in the given
	interval, finding the next element in the same staff and mapping a music
	element to one or more drawable instances.

	\sa CAScoreView, CADrawable
*/

template <typename T>
class CAKDTree {
public:
	CAKDTree();

	void addElement(T elt);
	T removeElement(double x, double y);

	QList<T> findInRange(double x, double y, double w=0, double h=0);
	QList<T> findInRange(QRect &area);
	T findNearestLeft(double x, bool timeBased=false, CADrawableContext *context=0, CAVoice *voice=0);
	T findNearestRight(double x, bool timeBased=false, CADrawableContext *context=0, CAVoice *voice=0);
	T findNearestUp(double y);
	T findNearestDown(double y);

	double getMaxX();
	double getMaxY();

	void clear(bool autoDelete=true);
	inline int size() { return _mapX.size(); }
	QList<T> list() { return _mapX.values(); }

private:
	//////////////////////
	// Basic properties //
	//////////////////////
	QMultiMap<double, T>         _mapX;        // List of all the drawable elements sorted by xPos()
	QMultiMap<double, T>         _mapXW;       // List of all the drawable elements sorted by xPos()+width()
	double                       _maxY;        // The largest Ypos()+height() value of any element

	void calculateMaxY();
};

/*!
	The default constructor.
*/
template <typename T>
CAKDTree<T>::CAKDTree() {
	_maxY = 0;
}

/*!
	Adds a drawable element \a elt to the tree.
*/
template <typename T>
void CAKDTree<T>::addElement(T elt) {
	_mapX.insertMulti(elt->xPos(), elt);
	
	if (elt->width()) {
		// regular music element
		_mapXW.insertMulti(elt->xPos()+elt->width(), elt);
	} else {
		// music element with unlimited width (e.g. staffs)
		_mapXW.insertMulti(std::numeric_limits<double>::max(), elt);
	}

	if (elt->yPos() + elt->height() > _maxY) {
		_maxY = elt->yPos() + elt->height();
	}
}

/*!
	Removes all elements from the tree.
	Also destroys the elements if \a autoDelete is true.
*/
template <typename T>
void CAKDTree<T>::clear(bool autoDelete) {
	if (autoDelete) {
		for (typename QMultiMap<double, T>::const_iterator it=_mapX.constBegin(); it!=_mapX.constEnd(); it++) {
			delete it.value();
		}
	}
	
	_mapX.clear();
	_mapXW.clear();
	
	_maxY = 0;
}

/*!
	Returns the list of elements present in the given rectangular area or an empty list if none found.
	Element is in the list, if the region only touches it - not neccessarily fits the whole in the region.
*/
template <typename T>
QList<T> CAKDTree<T>::findInRange(double x, double y, double w, double h) {
	QList<T> l;

	T rightMostElt = _mapX.lowerBound(x+w).value();
	for (typename QMultiMap<double, T>::const_iterator it=_mapXW.upperBound(x); (it!=_mapXW.end()) && (it.value()!=rightMostElt); it++) {
		if ( (// The object is normal and fits into the area
		      (it.value()->yPos() <= y+h) &&
		      (it.value()->yPos() + it.value()->height() >= y)) ||
		     (// The object is unlimited in width (eg. contexts)
			  (it.value()->width() == 0) &&                        
		      (it.value()->yPos() <= y+h) &&
		      (it.value()->yPos() + it.value()->height() >= y)) ||
		     (// The object is unlimited in height (eg. helper lines)
			  (it.value()->height() == 0))
		    ) {
			l << it.value();
		}
	}

	return l;
}

/*!
	This function is provided for convenience.
	Returns the list of elements present in the given rectangular area or an empty list if none found.
	Element is in the list, if the region only touches it - not neccessarily fits the whole in the region.
*/
template <typename T>
QList<T> CAKDTree<T>::findInRange(QRect &rect) {
	return findInRange(rect.x(), rect.y(), rect.width(), rect.height());
}

/*!
	Finds the nearest left element to the given coordinate and returns a pointer to it or 0 if none
	found. Left elements borders are taken into account.

	If \a timeBased is false (default), the lookup should be view-based - the nearest element is
	selected as it appears on the screen. If \a timeBased if true, the nearest element is selected
	according to the nearest start/end time.
*/
template <typename T>
T CAKDTree<T>::findNearestLeft(double x, bool timeBased, CADrawableContext *context, CAVoice *voice) {
	if (_mapX.size()==0) {
		return 0;
	}
	
	typename QMultiMap<double, T>::const_iterator it = _mapX.lowerBound(x); // returns constEnd, if not found
	while (it!=_mapX.constBegin() && (it==_mapX.constEnd() || it.value()->xPos()>=x)) {
		it--;
	}
	
	if (it.value()->xPos()>=x) {
		// no elements to the left at all
		return 0;
	}
	
	do {
		if (
			// compare contexts
			(!context  || it.value()->drawableContext() == context) &&
			// compare voices
			( !voice ||
				(
				// if the element isn't playable, see if it has the same context as the voice
				(!it.value()->musElement()->isPlayable() &&
					it.value()->musElement()->context() == voice->staff())
				||
				// if the element is playable, see if it has the exactly same voice
				(it.value()->musElement()->isPlayable() && 
					static_cast<CAPlayable*>(it.value()->musElement())->voice() == voice)
				)
			)
			) {
			return static_cast<T>(it.value());
		}
	} while ((it--)!=_mapX.constBegin());
	
	// no regular elements to the left exists
	return 0;
}

/*!
	Finds the nearest right element to the given coordinate and returns a pointer to it or 0 if none
	found. Left elements borders are taken into account.

	If \a timeBased is false (default), the lookup should be view-based - the nearest element is
	selected as it appears on the screen. If \a timeBased if true, the nearest element is selected
	according to the nearest start/end time.
*/
template <typename T>
T CAKDTree<T>::findNearestRight(double x, bool timeBased, CADrawableContext *context, CAVoice *voice) {
	typename QMultiMap<double, T>::const_iterator it = _mapX.upperBound(x);
	
	for (; it!=_mapX.constEnd(); it++) {
		if (
			// compare contexts
			(!context  || it.value()->drawableContext() == context) &&
			// compare voices
			( !voice ||
				(
				// if the element isn't playable, see if it has the same context as the voice
				(!it.value()->musElement()->isPlayable() &&
					it.value()->musElement()->context() == voice->staff())
				||
				// if the element is playable, see if it has the exactly same voice
				(it.value()->musElement()->isPlayable() && 
					static_cast<CAPlayable*>(it.value()->musElement())->voice() == voice)
				)
			)
			) {
			return static_cast<T>(it.value());
		}
	}
	
	// no elements to the right exists
	return 0;
}

/*!
	Finds the nearest upper element to the given coordinate and returns a pointer to it or 0 if none
	found. Top element border is taken into account.

	If \a timeBased is false (default), the lookup should be view-based - the nearest element is
	selected as it appears on the screen. If \a timeBased if true, the nearest element is selected
	according to the nearest start/end time.
*/
template <typename T>
T CAKDTree<T>::findNearestUp(double y) {
	if (_mapX.isEmpty())
		return 0;

	T elt=0;
	for (typename QMultiMap<double, T>::const_iterator it=_mapX.constBegin(); it!=_mapX.constEnd(); it++) {
		if ( ((!elt) || ((it.value()->yPos() + it.value()->height()) > (elt->yPos() + elt->height())))
		      && ((it.value()->yPos()+ it.value()->height()) < y) ) {
			elt = it.value();
		}
	}
	return elt;

}

/*!
	Finds the nearest lower element to the given coordinate and returns a pointer to it or 0 if none
	found. Top element border is taken into account.

	If \a timeBased is false (default), the lookup should be view-based - the nearest element is
	selected as it appears on the screen. If \a timeBased if true, the nearest element is selected
	according to the nearest start/end time.
*/
template <typename T>
T CAKDTree<T>::findNearestDown(double y) {
	if (_mapX.isEmpty())
		return 0;

	T elt=0;
	for (typename QMultiMap<double, T>::const_iterator it=_mapX.constBegin(); it!=_mapX.constEnd(); it++) {
		if ( ((!elt) || (it.value()->yPos() < elt->yPos())) && (it.value()->yPos() > y) ) {
			elt = it.value();
		}
	}

	return elt;
}

/*!
	Returns the max X coordinate of the end of the most-right element.
	This value is read from buffer, so the calculation time is constant.
*/
template <typename T>
double CAKDTree<T>::getMaxX() {
	for (typename QMultiMap<double, T>::const_iterator it=(--_mapXW.constEnd());
	     it!=_mapXW.constBegin();
	     it--) {
		if (it.key()!=std::numeric_limits<double>::max()) {
			// don't take contexts unlimited length into account
			return it.key();
		}
	}
	return 0;
}

/*!
	Returns the max Y coordinate of the end of the most-bottom element.
	This value is read from buffer, so the calculation time is constant.
*/
template <typename T>
double CAKDTree<T>::getMaxY() {
	return _maxY;
}

/*!
	Used internally for the maxX and maxY properties to update.
	Calculates the largest X and Y coordinates among all ends of elements and store it locally.
	This operation takes O(n) time complexity where n is number of elements in the tree.
*/
template <typename T>
void CAKDTree<T>::calculateMaxY() {
	_maxY = 0;
	for (typename QMultiMap<double, T>::iterator it=_mapX.begin(); it!=_mapX.end(); it++) {
		if (it.value()->yPos()+it.value()->height() > _maxY) {
			_maxY = it.value()->yPos()+it.value()->height();
		}
	}
}

#endif

/*!
	\fn int CAKDTree<T>::size()
	Returns the number of elements currently in the tree.
*/

/*!
	\fn CADrawable *CAKDTree<T>:at(int i)
	Returns the element with index \a i in the tree.
	If the element doesn't exist (eg. index out of bounds), returns 0.
*/

/*!
	\fn QList<T>& CAKDTree<T>:list()
	Returns the pointer to the list of all the elements.
*/
