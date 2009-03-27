/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef DRAWABLEACCIDENTAL_H_
#define DRAWABLEACCIDENTAL_H_

#include "drawable/drawablemuselement.h"

class CADrawableAccidental : public CADrawableMusElement {
	public:
		CADrawableAccidental(signed char accs, CAMusElement *musElement, CADrawableContext *drawableContext, double x, double y);
		~CADrawableAccidental();
		void draw(QPainter *p, CADrawSettings s);
		CADrawableAccidental *clone(CADrawableContext* newContext = 0);

	private:
		signed char _accs;
		double _centerX, _centerY; // easier to do clone(), otherwise not needed
};

#endif /* DRAWABLEACCIDENTAL_H_ */
