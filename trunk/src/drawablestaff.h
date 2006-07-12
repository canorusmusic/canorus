/** @file drawablestaff.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef DRAWABLESTAFF_H_
#define DRAWABLESTAFF_H_

#include "drawablecontext.h"

class CAStaff;

class CADrawableStaff : public CADrawableContext {
	public:
		CADrawableStaff(CAStaff* staff, int x, int y);
		void draw(QPainter *, const CADrawSettings s);
		CADrawableStaff *clone();
		inline CAStaff *staff() { return (CAStaff*)_context; }
};

#endif /*DRAWABLESTAFF_H_*/
