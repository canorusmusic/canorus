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
	
	int xMid() { return _xMid; }
	int yMid() { return _yMid; }
	int y1() { return _y1; }
	int y2() { return _y2; }
	inline void setXMid( int xMid ) { _xMid = xMid; }
	inline void setYMid( int yMid ) { _yMid = yMid; }
	inline void setY1( int y1 ) { _y1 = y1; }
	inline void setY2( int y2 ) { _y2 = y2; }
	
private:
	int min(int, int, int);
	int max(int, int, int);
	int _y1;
	int _xMid;
	int _yMid;
	int _y2;
};

#endif /* DRAWABLESLUR_H_ */
