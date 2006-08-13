/** @file drawablebarline.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef DRAWABLEBARLINE_H_
#define DRAWABLEBARLINE_H_

#include "drawable/drawablemuselement.h"

class CADrawableStaff;
class CABarline;

class CADrawableBarline : public CADrawableMusElement {
	public:
		CADrawableBarline(CABarline *m, CADrawableStaff *staff, int x, int y);
		~CADrawableBarline();
		
		void draw(QPainter *p, CADrawSettings s);
		CADrawableBarline *clone();
		inline CABarline *barline() { return (CABarline*)_musElement; }
		
	private:
		static const float BARLINE_WIDTH = 1.5;
};

#endif /*DRAWABLEBARLINE_H_*/
