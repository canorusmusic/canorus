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

class CADrawableBarline : public CADrawableMusElement {
	public:
		CADrawableBarline(CAMusElement *m, CADrawableStaff *staff, int x, int y);
		virtual ~CADrawableBarline();
};

#endif /*DRAWABLEBARLINE_H_*/
