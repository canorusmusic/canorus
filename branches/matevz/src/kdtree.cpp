#include "kdtree.h"

using namespace std;

CAKDTree::CAKDTree() {
}

/**
 * Add element in the tree.
 * 
 * @param CADrawable *elt Element to be added
 */
void CAKDTree::addElement(CADrawable *elt) {
	list_ << elt;
	
	if (elt->xPos() + elt->width() > maxX_)
		maxX_ = elt->xPos() + elt->width();
	if (elt->yPos() + elt->height() > maxY_)
		maxY_ = elt->yPos() + elt->height();		
}

/**
 * Remove given element.
 * 
 * @param CADrawable *elt Pointer to the element to be deleted
 * @return True, if element existed and has been deleted, false otherwise.
 */
bool CAKDTree::removeElement(CADrawable *elt) {
	delete elt;

	return list_.removeAll(elt);
}

/**
 * Remove element at given world coordinates.
 * 
 * @param int x X coordinate in world units
 * @param int y Y coordinate in world units
 * @return True if element existed and has been deleted, false otherwise.
 */
bool CAKDTree::removeElement(int x, int y) {
	for (int i=0; i<list_.size(); i++) {
		if (list_.at(i)->bBox().contains(x,y)) {
			list_.removeAt(i);
			calculateMaxXY();
			return true;
		}
	}
	
	return false;
}

/**
 * Return a list of elements inside the given area.
 * 
 * @param int x Top-left X coordinate of the area
 * @param int y Top-left Y coordinate of the are
 * @param int w Width of the area
 * @param int h Height of the area
 */
QList<CADrawable *>* CAKDTree::findInRange(int x, int y, int w, int h) {
	QList<CADrawable *> *l = new QList<CADrawable *>();

	for (int i=0; i<list_.size(); i++) {
		if ( (list_.at(i)->xPos() <= x+w) &&
		     (list_.at(i)->yPos() <= y+h) &&
		     (list_.at(i)->xPos()+list_.at(i)->width() >= x) &&
		     (list_.at(i)->yPos()+list_.at(i)->height() >= y) )
			*l << list_.at(i);
	}

	return l;	
}

/**
 * This is an overloaded member function, provided for convenience.
 * 
 * @param QRect *rect Pointer the given area 
 */
QList<CADrawable *>* CAKDTree::findInRange(QRect *rect) {
	return findInRange(rect->x(), rect->y(), rect->width(), rect->height());
}

/**
 * Return the largest X coordinate among all ends of elements.
 * This is useful for determining the document edges.
 * This value is buffered, it requires O(1) time.
 * 
 * @return X coordinate in world units
 */
int CAKDTree::getMaxX() {
	return maxX_;
}

/**
 * Return the largest Y coordinate among all ends of elements.
 * This is useful for determining the document edges.
 * This value is buffered, it requires O(1) time.
 * 
 * @return Y coordinates in world units
 */
int CAKDTree::getMaxY() {
	return maxY_;
}

/**
 * Calculate the largest X and Y coordinates among all ends of
 * elements and store it locally.
 * This operation takes O(n) time.
 */
void CAKDTree::calculateMaxXY() {
	maxX_ = 0;
	maxY_ = 0;
	for (int i=0; i<list_.size(); i++) {
		if (list_.at(i)->xPos() + list_.at(i)->width() > maxX_)
			maxX_ = list_.at(i)->xPos() + list_.at(i)->width();
		if (list_.at(i)->yPos() + list_.at(i)->height() > maxY_)
			maxY_ = list_.at(i)->yPos() + list_.at(i)->height();
	}
}
