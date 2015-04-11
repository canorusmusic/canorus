/*!
	Copyright (c) 2008-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef DRAWABLETUPLET_H_
#define DRAWABLETUPLET_H_

#include "layout/drawablemuselement.h"
#include "score/tuplet.h"

class CADrawableTuplet: public CADrawableMusElement {
public:
	CADrawableTuplet( CATuplet *tuplet, CADrawableContext *c, double x1, double y1, double x2, double y2 );
	virtual ~CADrawableTuplet();

	CATuplet *tuplet() { return static_cast<CATuplet*>(_musElement); }

	void draw(QPainter *p, const CADrawSettings s);
	CADrawableTuplet *clone(CADrawableContext* newContext = 0);

	inline double x1() { return _x1; }
	inline double y1() { return _y1; }
	inline double x2() { return _x2; }
	inline double y2() { return _y2; }
	inline void setX1( double x1 ) { _x1 = x1; }
	inline void setY1( double y1 ) { _y1 = y1; }
	inline void setX2( double x2 ) { _x2 = x2; }
	inline void setY2( double y2 ) { _y2 = y2; }

private:
	double _x1;
	double _x2;
	double _y1;
	double _y2;
};

#endif /* DRAWABLETUPLET_H_ */
