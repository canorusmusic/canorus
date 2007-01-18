/* 
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#ifndef KDTREE_H
#define KDTREE_H

#include <QList>
#include <QRect>

class CADrawable;
class CADrawableContext;
class CAVoice;

/*class Node {
public:
	CADrawable *elt;	         // pointer to the element
	CADrawable *right, *left; // pointer to the left/right son
	int x1,y1,x2,y2;	 // element keys	
};*/

class CAKDTree {
public:
	CAKDTree();
	
	void addElement(CADrawable *elt);
	bool removeElement(CADrawable *elt);
	CADrawable* removeElement(int x, int y, bool autoDelete=true);
	void import(CAKDTree *tree);
	
	QList<CADrawable *> findInRange(int x, int y, int w=0, int h=0);
	QList<CADrawable *> findInRange(QRect *area);
	CADrawable *findNearestLeft(int x, bool timeBased=false, CADrawableContext *context=0, CAVoice *voice=0);
	CADrawable *findNearestRight(int x, bool timeBased=false, CADrawableContext *context=0, CAVoice *voice=0);
	CADrawable *findNearestUp(int y);
	CADrawable *findNearestDown(int y);
	
	int getMaxX();
	int getMaxY();
	
	void clear(bool autoDelete=true);
	inline int size() { return _list.size(); }
	CADrawable *at(int i) { return _list[i]; }
	QList<CADrawable *>& list() { return _list; }
	
private:
	//////////////////////
	// Basic properties //
	//////////////////////
	QList<CADrawable *> _list;	// List of all the music elements.
	int _maxX, _maxY;	// MaxX and MaxY bound of the end of the most-right and the most-bottom elements.
	
	void calculateMaxXY();
};
#endif
