/** @file drawableclef.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "drawablemuselement.h"

#ifndef DRAWABLECLEF_H_
#define DRAWABLECLEF_H_

class CAClef;
class CADrawableStaff;

class CADrawableClef : public CADrawableMusElement {
	public:
		/**
		 * Default constructor.
		 * 
		 * @param clef Pointer to the logical CAClef.
		 * @param x X coordinate of the left-margin of the clef.
		 * @param y Y coordinate of the top of the staff. (WARNING! Not top of the clef!)
		 */
		CADrawableClef(CAClef *clef, CADrawableStaff *drawableStaff, int x, int y);
		void draw(QPainter *p, CADrawSettings s);
		CADrawableClef *clone();
		inline CAClef *clef() { return (CAClef*)_musElement; }
};

#endif /*DRAWABLECLEF_H_*/
