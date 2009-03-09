/*!
 * Copyright (c) 2008, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 *
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#ifndef DRAWABLETUPLET_H_
#define DRAWABLETUPLET_H_

#include "drawable/drawablemuselement.h"
#include "core/tuplet.h"

class CADrawableTuplet: public CADrawableMusElement {
public:
	CADrawableTuplet( CATuplet *tuplet, CADrawableContext *c, int x1, int y1, int x2, int y2 );
	virtual ~CADrawableTuplet();

	CATuplet *tuplet() { return static_cast<CATuplet*>(_musElement); }

	void draw(QPainter *p, const CADrawSettings s);
	CADrawableTuplet *clone(CADrawableContext* newContext = 0);

	inline int x1() { return _x1; }
	inline int y1() { return _y1; }
	inline int x2() { return _x2; }
	inline int y2() { return _y2; }
	inline void setX1(int x1) { _x1 = x1; }
	inline void setY1( int y1 ) { _y1 = y1; }
	inline void setX2(int x2) { _x2 = x2; }
	inline void setY2( int y2 ) { _y2 = y2; }

private:
	int _x1;
	int _x2;
	int _y1;
	int _y2;
};

#endif /* DRAWABLETUPLET_H_ */
