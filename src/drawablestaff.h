/** @file drawablestaff.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef DRAWABLESTAFF_H_
#define DRAWABLESTAFF_H_

#include "drawable.h"

class CAStaff;

class CADrawableStaff : public CADrawable {
	public:
		CADrawableStaff(CAStaff* staff, int x, int y);
		void draw(QPainter *, const CADrawSettings s);
		CADrawableStaff *clone();
		
	private:
		CAStaff *_staff;
};

#endif /*DRAWABLESTAFF_H_*/
