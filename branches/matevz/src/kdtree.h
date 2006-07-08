#ifndef KDTREE_H
#define KDTREE_H

#include <vector>
#include <QList>
#include <QRect>

#include "drawable.h"

using namespace std;

/*class Node {
	public:
		CADrawable *elt;	         //pointer to the element
		CADrawable *right, *left; //pointer to the left/right son
		int x1,y1,x2,y2;	 //element keys
		
};*/

/**
 * kd-tree usually used for description of the music elements on the canvas.
 * Used for fast insertion/finding of a list of elements in the given region.
 * Currently, an ordinary vector is used. No tree structure present yet,
 * so all the operations are O(n)!!
 */
class CAKDTree {
	public:
		CAKDTree();
		QList<CADrawable *>* findInRange(int x1, int y1, int x2, int y2);
		QList<CADrawable *>* findInRange(QRect *area);
		int getMaxX();
		int getMaxY();
		void addElement(CADrawable *elt);
		bool removeElement(CADrawable *elt);
		bool removeElement(int x, int y);
		void clear() { list_.clear(); }
	
	private:
		void calculateMaxXY();
		
		QList<CADrawable *> list_;
		int maxX_, maxY_;
};
#endif

