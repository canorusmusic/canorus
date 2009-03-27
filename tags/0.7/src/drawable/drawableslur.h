/*!
	Copyright (c) 2006-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef DRAWABLESLUR_H_
#define DRAWABLESLUR_H_

#include "drawable/drawablemuselement.h"
#include "core/slur.h"

class CASlur;

class CADrawableSlur : public CADrawableMusElement {
public:
	CADrawableSlur( CASlur *slur, CADrawableContext *c, double x1, double y1, double xMid, double yMid, double x2, double y2 );
	virtual ~CADrawableSlur();

	CASlur *slur() { return static_cast<CASlur*>(_musElement); }

	void draw(QPainter *p, const CADrawSettings s);
	CADrawableSlur *clone(CADrawableContext* newContext = 0);

	inline double x1() { return _x1; }
	inline double y1() { return _y1; }
	inline double xMid() { return _xMid; }
	inline double yMid() { return _yMid; }
	inline double x2() { return _x2; }
	inline double y2() { return _y2; }
	inline void setX1(double x1) { _x1 = x1; updateGeometry(); }
	inline void setY1( double y1 ) { _y1 = y1; updateGeometry(); }
	inline void setXMid( double xMid ) { _xMid = xMid; updateGeometry(); }
	inline void setYMid( double yMid ) { _yMid = yMid; updateGeometry(); }
	inline void setX2( double x2) { _x2 = x2; updateGeometry(); }
	inline void setY2( double y2 ) { _y2 = y2; updateGeometry(); }

private:
	void updateGeometry();

	double min(double, double, double);
	double max(double, double, double);
	double _x1;
	double _y1;
	double _xMid;
	double _yMid;
	double _x2;
	double _y2;
};

#endif /* DRAWABLESLUR_H_ */
