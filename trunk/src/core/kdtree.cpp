/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICNESE.GPL for details.
*/

#include "drawable/drawable.h"
#include "drawable/drawablemuselement.h"

#include "core/kdtree.h"
#include "core/muselement.h"
#include "core/playable.h"
#include "core/voice.h"

/*!
	\class CAKDTree
	kd-tree is usually used for description of the music elements on the canvas.
	It's used for fast insertion/lookup of a list of elements in the given region.
	Each canvas has its own kd-tree of drawable elements. One for contexts and the other for music
	elements.
	
	See http://en.wikipedia.org/wiki/Kd_tree
	
	\todo Currently, an ordinary vector is used. No tree structure present yet, so all the operations are slow - O(n).
	
	\sa CAScoreViewPort, CADrawable
*/

/*!
	The default constructor.
*/
CAKDTree::CAKDTree() {
	_maxX = 0;
	_maxY = 0;
}

/*!
	Adds a drawable element \a elt to the tree.
*/
void CAKDTree::addElement(CADrawable *elt) {
	_list << elt;
	
	if (elt->xPos() + elt->width() > _maxX)
		_maxX = elt->xPos() + elt->width();
	if (elt->yPos() + elt->height() > _maxY)
		_maxY = elt->yPos() + elt->height();		
}

/*!
	Removes the given element \a elt from the tree.
	Returns true, if the given element was found and deleted; otherwise false.
*/
bool CAKDTree::removeElement(CADrawable *elt) {
	delete elt;

	return _list.removeAll(elt);
}

/*!
	Removes the first element at the given \a x and \a y absolute world coordinates and returns the
	pointer to this element, if successful. Destroys element, if \a autoDelete is true.
	Returns pointer to the deleted element or null, if no element found at the given coordinates.
*/
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

/*!
	Removes all elements from the tree.
	Also destroys the elements if \a autoDelete is true.
*/
void CAKDTree::clear(bool autoDelete) {
	if (autoDelete) {
		for (int i=0; i<_list.size(); i++)
			delete _list[i];
	}
	
	_list.clear();
}

/*!
	Returns the list of elements present in the given rectangular area or an empty list if none found.
*/
QList<CADrawable *> CAKDTree::findInRange(int x, int y, int w, int h) {
	QList<CADrawable *> l;

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
			l << _list[i];
	}

	return l;	
}

/*!
	This function is provided for convenience.
	Returns the list of elements present in the given rectangular area or an empty list if none found.
*/
QList<CADrawable *> CAKDTree::findInRange(QRect *rect) {
	return findInRange(rect->x(), rect->y(), rect->width(), rect->height());
}

/*!
	Finds the nearest left element to the given coordinate and returns a pointer to it or 0 if none
	found. Left elements borders are taken into account.
	
	If \a timeBased is false (default), the lookup should be view-based - the nearest element is
	selected as it appears on the screen. If \a timeBased if true, the nearest element is selected
	according to the nearest start/end time.
*/
CADrawable* CAKDTree::findNearestLeft(int x, bool timeBased, CADrawableContext *context, CAVoice *voice) {
	if (_list.isEmpty())
		return 0;
		
	CADrawable *elt=0;
	QList<CADrawable *>::const_iterator i;
	for (i = _list.constBegin(); i != _list.constEnd(); i++) {
		if ( ( !elt || (timeBased?(*i)->xPosOrig():(*i)->xPos()) > (timeBased?elt->xPosOrig():elt->xPos()) ) && // element's X is lesser than the already found element's X
		     ( ( timeBased?(*i)->xPosOrig():(*i)->xPos() ) < x) && // element's X is lesser than the given X
		     ( !context  || ((CADrawableMusElement*)(*i))->drawableContext() == context ) && // compare contexts
		     ( !voice || // compare voices
		         !((CADrawableMusElement*)(*i))->musElement()->isPlayable() && // if the element isn't playable, see if it has the same context as the voice
		         ((CADrawableMusElement*)(*i))->musElement()->context() == voice->staff()
		         || 
		         ((CADrawableMusElement*)(*i))->musElement()->isPlayable() && // if the element is playable, see if it has the exactly same voice
		         ((CAPlayable*)((CADrawableMusElement*)(*i))->musElement())->voice() == voice
		     )
		   ) {
			elt = *i;
		}
	}
		
	return elt;
}

/*!
	Finds the nearest right element to the given coordinate and returns a pointer to it or 0 if none
	found. Left elements borders are taken into account.
	
	If \a timeBased is false (default), the lookup should be view-based - the nearest element is
	selected as it appears on the screen. If \a timeBased if true, the nearest element is selected
	according to the nearest start/end time.
*/
CADrawable* CAKDTree::findNearestRight(int x, bool timeBased, CADrawableContext *context, CAVoice *voice) {
	if (_list.isEmpty())
		return 0;
		
	CADrawable *elt=0;
	QList<CADrawable *>::const_iterator i;
	for (i = _list.constBegin(); i != _list.constEnd(); i++) {
		if ( ( !elt || (timeBased?(*i)->xPosOrig():(*i)->xPos()) < (timeBased?elt->xPosOrig():elt->xPos()) ) && // element's X is lesser than the already found element's X
		     ( ( timeBased?(*i)->xPosOrig():(*i)->xPos() ) > x) && // element's X is lesser than the given X
		     ( !context  || ((CADrawableMusElement*)(*i))->drawableContext() == context ) && // compare contexts
		     ( !voice || // compare voices
		         !((CADrawableMusElement*)(*i))->musElement()->isPlayable() && // if the element isn't playable, see if it has the same context as the voice
		         ((CADrawableMusElement*)(*i))->musElement()->context() == voice->staff()
		         || 
		         ((CADrawableMusElement*)(*i))->musElement()->isPlayable() && // if the element is playable, see if it has the exactly same voice
		         ((CAPlayable*)((CADrawableMusElement*)(*i))->musElement())->voice() == voice
		     )
		   ) {
			elt = *i;
		}
	}
	
	return elt;
}

/*!
	Finds the nearest upper element to the given coordinate and returns a pointer to it or 0 if none
	found. Top element border is taken into account.
	
	If \a timeBased is false (default), the lookup should be view-based - the nearest element is
	selected as it appears on the screen. If \a timeBased if true, the nearest element is selected
	according to the nearest start/end time.
*/
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

/*!
	Finds the nearest lower element to the given coordinate and returns a pointer to it or 0 if none
	found. Top element border is taken into account.
	
	If \a timeBased is false (default), the lookup should be view-based - the nearest element is
	selected as it appears on the screen. If \a timeBased if true, the nearest element is selected
	according to the nearest start/end time.
*/
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

/*!
	Returns the max X coordinate of the end of the most-right element.
	This value is read from buffer, so the calculation time is constant.
*/
int CAKDTree::getMaxX() {
	return _maxX;
}

/*!
	Returns the max Y coordinate of the end of the most-bottom element.
	This value is read from buffer, so the calculation time is constant.
*/
int CAKDTree::getMaxY() {
	return _maxY;
}

/*!
	Used internally for the maxX and maxY properties to update.
	Calculates the largest X and Y coordinates among all ends of elements and store it locally.
	This operation takes O(n) time complexity where n is number of elements in the tree.
*/
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

/*!
	Imports all the elements from the given \a tree.
	It actually merges the given tree with this one.
*/
void CAKDTree::import(CAKDTree *tree) {
	for (int i=0; i<tree->list().size(); i++)
		_list += tree->list().at(i)->clone();
	
	calculateMaxXY();
}

/*!
	\fn CAKDTree::size()
	Returns the number of elements currently in the tree.
*/

/*!
	\fn CAKDTree:at(int i)
	Returns the element with index \a i in the tree.
	If the element doesn't exist (eg. index out of bounds), returns 0.
*/

/*!
	\fn CAKDTree:list()
	Returns the pointer to the list of all the elements.
*/
