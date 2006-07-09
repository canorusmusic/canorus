/** @file kdtree.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef KDTREE_H
#define KDTREE_H

#include <QList>
#include <QRect>

class CADrawable;

/*class Node {
	public:
		CADrawable *elt;	         //pointer to the element
		CADrawable *right, *left; //pointer to the left/right son
		int x1,y1,x2,y2;	 //element keys
		
};*/

/**
 * kd-tree usually used for description of the music elements on the canvas.
 * Used for fast insertion/lookup of a list of elements in the given region.
 * 
 * @todo Currently, an ordinary vector is used. No tree structure present yet, so all the operations are slow - O(n).
 */
class CAKDTree {
	public:
		/**
		 * The default constructor.
		 */
		CAKDTree();
		
		/**
 		 * Add element to the tree.
 		 * 
 		 * @param elt Element to be added.
 		 */
		void addElement(CADrawable *elt);
		
		/**
 		 * Remove the given element from the tree.
 		 * 
 		 * @param elt Pointer to the element to be deleted.
 		 * @return True, if element existed and has been deleted, false otherwise.
 		 */
		bool removeElement(CADrawable *elt);
		
		/**
		 * Remove element at given world coordinates.
		 * 
		 * @param x X coordinate in absolute world units.
		 * @param y Y coordinate in absolute world units.
		 * @return True, if element existed and has been deleted, false otherwise.
		 */
		bool removeElement(int x, int y);

		/**
		 * Return the list of elements present in the given rectangular area.
		 * 
		 * @param x Top-left X coordinate of the area in absolute world units.
		 * @param y Top-left Y coordinate of the area in absolute world units.
		 * @param w Width of the area in absolute world units.
		 * @param w Width of the area in absolute world units.
		 * @return Pointer to QList of the elements inside the given area.
		 */
		QList<CADrawable *>* findInRange(int x, int y, int w, int h);
		
		/**
		 * This is an overloaded member function, provided for convenience.
		 * 
		 * Return the list of elements present in the given rectangular area.
		 * 
		 * @param area QRect of the given area in absolute world units.
		 * @return Pointer to QList of the elements inside the given area.
		 */
		QList<CADrawable *>* findInRange(QRect *area);

		/**
		 * Return the max X coordinate of the end of the most-right element.
		 * This value is read from buffer, so the time complexity is O(1)!
		 * 
		 * @return X coordinate of the end of the most-right element in absolute world units.
		 */
		int getMaxX();

		/**
		 * Return the max Y coordinate of the end of the most-bottom element.
		 * This value is read from buffer, so the time complexity is O(1)!
		 * 
		 * @return Y coordinate of the end of the most-bototom element in absolute world units.
		 */
		int getMaxY();

		/**
		 * Remove all elements from the list.
		 */
		void clear() { _list.clear(); }
	
	private:
		////////////////////////////////////////////////
		//Basic properties
		////////////////////////////////////////////////
		QList<CADrawable *> _list;	//List of all the music elements.
		int _maxX, _maxY;	//MaxX and MaxY bound of the end of the most-right and the most-bottom elements.
		
		/**
		 * Used internally for the maxX and maxY properties to update.
		 * Calculate the largest X and Y coordinates among all ends of elements and store it locally.
		 * This operation takes O(n) time complexity.
		 */
		void calculateMaxXY();
};
#endif

