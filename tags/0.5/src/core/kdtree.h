/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef KDTREE_H
#define KDTREE_H

#include <QList>
#include <QRect>

#include "drawable/drawable.h"
#include "drawable/drawablecontext.h"
#include "drawable/drawablemuselement.h"

#include "core/kdtree.h"
#include "core/muselement.h"
#include "core/playable.h"
#include "core/voice.h"

/*class Node {
public:
	CADrawable *elt;	      // pointer to the element
	CADrawable *right, *left; // pointer to the left/right son
	int x1,y1,x2,y2;	      // element keys	
};*/

template <typename T>
class CAKDTree {
public:
	CAKDTree();
	
	void addElement(T elt);
	bool removeElement(T elt);
	T removeElement(int x, int y, bool autoDelete=true);
	void import(CAKDTree *tree);
	
	QList<T> findInRange(int x, int y, int w=0, int h=0);
	QList<T> findInRange(QRect &area);
	T findNearestLeft(int x, bool timeBased=false, CADrawableContext *context=0, CAVoice *voice=0);
	T findNearestRight(int x, bool timeBased=false, CADrawableContext *context=0, CAVoice *voice=0);
	T findNearestUp(int y);
	T findNearestDown(int y);
	
	int getMaxX();
	int getMaxY();
	
	void clear(bool autoDelete=true);
	inline int size() { return _list.size(); }
	T at(int i) { return _list[i]; }
	QList<T>& list() { return _list; }
	
private:
	//////////////////////
	// Basic properties //
	//////////////////////
	QList<T> _list;	       // List of all the drawable elements.
	int _maxX, _maxY;	   // MaxX and MaxY bound of the end of the most-right and the most-bottom elements.
	
	void calculateMaxXY();
};

/*!
	\class CAKDTree
	\brief Space partitioning structure for fast access to drawable elements on canvas
	
	kd-tree is usually used for description of the music elements on the canvas.
	It's used for fast insertion/lookup of a list of elements in the given region.
	Each canvas has its own kd-tree of drawable elements. One for contexts and the other for music
	elements.
	
	Canorus kd-tree is a template class which requires derivates of CADrawable base class elements to work.
	
	See http://en.wikipedia.org/wiki/Kd_tree
	
	\todo Currently, an ordinary vector is used. No tree structure present yet, so all the operations are slow - O(n).
	
	\sa CAScoreViewPort, CADrawable
*/

/*!
	The default constructor.
*/
template <typename T>
CAKDTree<T>::CAKDTree() {
	_maxX = 0;
	_maxY = 0;
}

/*!
	Adds a drawable element \a elt to the tree.
*/
template <typename T>
void CAKDTree<T>::addElement(T elt) {
	_list << elt;
	
	if (static_cast<CADrawable*>(elt)->xPos() + static_cast<CADrawable*>(elt)->width() > _maxX)
		_maxX = static_cast<CADrawable*>(elt)->xPos() + static_cast<CADrawable*>(elt)->width();
	if (static_cast<CADrawable*>(elt)->yPos() + static_cast<CADrawable*>(elt)->height() > _maxY)
		_maxY = static_cast<CADrawable*>(elt)->yPos() + static_cast<CADrawable*>(elt)->height();		
}

/*!
	Removes the given element \a elt from the tree.
	Returns true, if the given element was found and deleted; otherwise false.
*/
template <typename T>
bool CAKDTree<T>::removeElement(T elt) {
	delete elt;

	return _list.removeAll(elt);
}

/*!
	Removes the first element at the given \a x and \a y absolute world coordinates and returns the
	pointer to this element, if successful. Destroys element, if \a autoDelete is true.
	Returns pointer to the deleted element or null, if no element found at the given coordinates.
*/
template <typename T>
T CAKDTree<T>::removeElement(int x, int y, bool autoDelete) {
	T elt;
	for (int i=0; i<_list.size(); i++) {
		if (static_cast<CADrawable*>(_list.at(i))->bBox().contains(x,y)) {
			elt = _list[i];
			_list.removeAt(i);
			if (autoDelete) delete elt;
			
			calculateMaxXY();
			
			return elt;
		}
	}
	
	return 0;
}

/*!
	Removes all elements from the tree.
	Also destroys the elements if \a autoDelete is true.
*/
template <typename T>
void CAKDTree<T>::clear(bool autoDelete) {
	if (autoDelete) {
		for (int i=0; i<_list.size(); i++)
			delete _list[i];
	}
	
	_list.clear();
}
#include <iostream>
/*!
	Returns the list of elements present in the given rectangular area or an empty list if none found.
	Element is in the list, if the region only touches it - not neccessarily fits the whole in the region.
*/
template <typename T>
QList<T> CAKDTree<T>::findInRange(int x, int y, int w, int h) {
	QList<T> l;

	for (int i=0; i<_list.size(); i++) {
		if ( ((static_cast<CADrawable*>(_list[i])->xPos() <= x+w) &&                       // The object is normal and fits into the area
		      (static_cast<CADrawable*>(_list[i])->yPos() <= y+h) &&
		      (static_cast<CADrawable*>(_list[i])->xPos() + static_cast<CADrawable*>(_list[i])->width() >= x) &&
		      (static_cast<CADrawable*>(_list[i])->yPos() + static_cast<CADrawable*>(_list[i])->height() >= y)) ||
		     ((static_cast<CADrawable*>(_list[i])->width() == 0) &&                        // The object is unlimited in width (eg. contexts)
		      (static_cast<CADrawable*>(_list[i])->yPos() <= y+h) &&
		      (static_cast<CADrawable*>(_list[i])->yPos() + static_cast<CADrawable*>(_list[i])->height() >= y)) ||
		     ((static_cast<CADrawable*>(_list[i])->height() == 0) &&                       // The object is unlimited in height (eg. helper lines)
		      (static_cast<CADrawable*>(_list[i])->xPos() <= x+w) &&
		      (static_cast<CADrawable*>(_list[i])->xPos() + static_cast<CADrawable*>(_list[i])->width() >= x))
		    ) {
			l << _list[i];
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
T CAKDTree<T>::findNearestLeft(int x, bool timeBased, CADrawableContext *context, CAVoice *voice) {
	if (_list.isEmpty())
		return 0;
		
	CADrawable *elt=0;
	int i;
	for (i=0; i < _list.size(); i++) {
		if ( ( !elt || (timeBased?(static_cast<CADrawable*>(_list[i]))->xPosOrig():(static_cast<CADrawable*>(_list[i]))->xPos()) > (timeBased?elt->xPosOrig():elt->xPos()) ) && // element's X is lesser than the already found element's X
		     ( ( timeBased?(static_cast<CADrawable*>(_list[i]))->xPosOrig():(static_cast<CADrawable*>(_list[i]))->xPos() ) < x) && // element's X is lesser than the given X
		     ( !context  || static_cast<CADrawableMusElement*>(_list[i])->drawableContext() == context ) && // compare contexts
		     ( !voice || // compare voices
		       !(static_cast<CADrawableMusElement*>(_list[i]))->musElement() ||
		         !(static_cast<CADrawableMusElement*>(_list[i]))->musElement()->isPlayable() && // if the element isn't playable, see if it has the same context as the voice
		         (static_cast<CADrawableMusElement*>(_list[i]))->musElement()->context() == voice->staff()
		         || 
		         (static_cast<CADrawableMusElement*>(_list[i]))->musElement()->isPlayable() && // if the element is playable, see if it has the exactly same voice
		         static_cast<CAPlayable*>(static_cast<CADrawableMusElement*>(_list[i])->musElement())->voice() == voice
		     )
		   ) {
			elt = static_cast<CADrawable*>(_list[i]);
		}
	}
		
	return static_cast<T>(elt);
}

/*!
	Finds the nearest right element to the given coordinate and returns a pointer to it or 0 if none
	found. Left elements borders are taken into account.
	
	If \a timeBased is false (default), the lookup should be view-based - the nearest element is
	selected as it appears on the screen. If \a timeBased if true, the nearest element is selected
	according to the nearest start/end time.
*/
template <typename T>
T CAKDTree<T>::findNearestRight(int x, bool timeBased, CADrawableContext *context, CAVoice *voice) {
	if (_list.isEmpty())
		return 0;
		
	CADrawable *elt=0;
	int i;
	for (i=0; i < _list.size(); i++) {
		if ( ( !elt || (timeBased?(static_cast<CADrawable*>(_list[i]))->xPosOrig():(static_cast<CADrawable*>(_list[i]))->xPos()) < (timeBased?elt->xPosOrig():elt->xPos()) ) && // element's X is greater than the already found element's X
		     ( ( timeBased?(static_cast<CADrawable*>(_list[i]))->xPosOrig():(static_cast<CADrawable*>(_list[i]))->xPos() ) > x) && // element's X is lesser than the given X
		     ( !context  || static_cast<CADrawableMusElement*>(_list[i])->drawableContext() == context ) && // compare contexts
		     ( !voice || // compare voices
		       !(static_cast<CADrawableMusElement*>(_list[i]))->musElement() ||
		         !(static_cast<CADrawableMusElement*>(_list[i]))->musElement()->isPlayable() && // if the element isn't playable, see if it has the same context as the voice
		         (static_cast<CADrawableMusElement*>(_list[i]))->musElement()->context() == voice->staff()
		         || 
		         (static_cast<CADrawableMusElement*>(_list[i]))->musElement()->isPlayable() && // if the element is playable, see if it has the exactly same voice
		         static_cast<CAPlayable*>(static_cast<CADrawableMusElement*>(_list[i])->musElement())->voice() == voice
		     )
		   ) {
			elt = static_cast<CADrawable*>(_list[i]);
		}
	}
	
	return static_cast<T>(elt);
}

/*!
	Finds the nearest upper element to the given coordinate and returns a pointer to it or 0 if none
	found. Top element border is taken into account.
	
	If \a timeBased is false (default), the lookup should be view-based - the nearest element is
	selected as it appears on the screen. If \a timeBased if true, the nearest element is selected
	according to the nearest start/end time.
*/
template <typename T>
T CAKDTree<T>::findNearestUp(int y) {
	if (_list.isEmpty())
		return 0;
		
	CADrawable *elt=0;
	int i;
	for (i=0; i<_list.size(); i++) {
		if ( ((!elt) || ((static_cast<CADrawable*>(_list[i])->yPos() + static_cast<CADrawable*>(_list[i])->height()) > (elt->yPos() + elt->height())))
		      && ((static_cast<CADrawable*>(_list[i])->yPos()+ static_cast<CADrawable*>(_list[i])->height()) < y) ) {
			elt = static_cast<CADrawable*>(_list[i]);
		}
	}
	return static_cast<T>(elt);
	
}

/*!
	Finds the nearest lower element to the given coordinate and returns a pointer to it or 0 if none
	found. Top element border is taken into account.
	
	If \a timeBased is false (default), the lookup should be view-based - the nearest element is
	selected as it appears on the screen. If \a timeBased if true, the nearest element is selected
	according to the nearest start/end time.
*/
template <typename T>
T CAKDTree<T>::findNearestDown(int y) {
	if (_list.isEmpty())
		return 0;
		
	CADrawable *elt=0;
	int i;
	for (i=0; i<_list.size(); i++) {
		if ( ((!elt) || (static_cast<CADrawable*>(_list[i])->yPos() < elt->yPos())) && (static_cast<CADrawable*>(_list[i])->yPos() > y) ) {
			elt = static_cast<CADrawable*>(_list[i]);
		}
	}
	
	return static_cast<T>(elt);
}

/*!
	Returns the max X coordinate of the end of the most-right element.
	This value is read from buffer, so the calculation time is constant.
*/
template <typename T>
int CAKDTree<T>::getMaxX() {
	return _maxX;
}

/*!
	Returns the max Y coordinate of the end of the most-bottom element.
	This value is read from buffer, so the calculation time is constant.
*/
template <typename T>
int CAKDTree<T>::getMaxY() {
	return _maxY;
}

/*!
	Used internally for the maxX and maxY properties to update.
	Calculates the largest X and Y coordinates among all ends of elements and store it locally.
	This operation takes O(n) time complexity where n is number of elements in the tree.
*/
template <typename T>
void CAKDTree<T>::calculateMaxXY() {
	_maxX = 0;
	_maxY = 0;
	for (int i=0; i<_list.size(); i++) {
		if (static_cast<CADrawable*>(_list[i])->xPos() + static_cast<CADrawable*>(_list[i])->width() > _maxX)
			_maxX = static_cast<CADrawable*>(_list[i])->xPos() + static_cast<CADrawable*>(_list[i])->width();
		if (static_cast<CADrawable*>(_list[i])->yPos() + static_cast<CADrawable*>(_list[i])->height() > _maxY)
			_maxY = static_cast<CADrawable*>(_list[i])->yPos() + static_cast<CADrawable*>(_list[i])->height();
	}
}

/*!
	Imports all the elements from the given \a tree.
	It actually merges the given tree with this one.
*/
template <typename T>
void CAKDTree<T>::import(CAKDTree *tree) {
	for (int i=0; i<tree->list().size(); i++)
		_list += tree->list().at(i)->clone();
	
	calculateMaxXY();
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
