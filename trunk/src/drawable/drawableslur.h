/*!
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#ifndef DRAWABLESLUR_H_
#define DRAWABLESLUR_H_

#include "drawable/drawablemuselement.h"
#include "core/slur.h"

class CASlur;

class CADrawableSlur : public CADrawableMusElement {
public:
	CADrawableSlur( CASlur *slur, CADrawableContext *c, int x1, int y1, int xMid, int yMid, int x1, int y1 );
	virtual ~CADrawableSlur();
	
	CASlur *slur() { return static_cast<CASlur*>(_musElement); }
	
	void draw(QPainter *p, const CADrawSettings s);
	CADrawableSlur *clone(CADrawableContext* newContext = 0);
	
	inline int x1() { return _x1; }
	inline int y1() { return _y1; }
	inline int xMid() { return _xMid; }
	inline int yMid() { return _yMid; }
	inline int x2() { return _x2; }
	inline int y2() { return _y2; }
	inline void setX1(int x1) { _x1 = x1; updateGeometry(); }
	inline void setY1( int y1 ) { _y1 = y1; updateGeometry(); }
	inline void setXMid( int xMid ) { _xMid = xMid; updateGeometry(); }
	inline void setYMid( int yMid ) { _yMid = yMid; updateGeometry(); }
	inline void setX2(int x2) { _x2 = x2; updateGeometry(); }
	inline void setY2( int y2 ) { _y2 = y2; updateGeometry(); }
	
private:
	void updateGeometry();
	
	int min(int, int, int);
	int max(int, int, int);
	int _x1;
	int _y1;
	int _xMid;
	int _yMid;
	int _x2;
	int _y2;
};

#endif /* DRAWABLESLUR_H_ */
