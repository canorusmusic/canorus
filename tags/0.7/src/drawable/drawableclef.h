/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "drawable/drawablemuselement.h"

#ifndef DRAWABLECLEF_H_
#define DRAWABLECLEF_H_

class CAClef;
class CADrawableStaff;

class CADrawableClef : public CADrawableMusElement {
public:
	CADrawableClef(CAClef *clef, CADrawableStaff *drawableStaff, double x, double y);

	void draw(QPainter *p, CADrawSettings s);
	CADrawableClef *clone(CADrawableContext* newContext = 0);
	inline CAClef *clef() { return (CAClef*)_musElement; }

	static const int CLEF_EIGHT_SIZE;
};

#endif /* DRAWABLECLEF_H_ */
