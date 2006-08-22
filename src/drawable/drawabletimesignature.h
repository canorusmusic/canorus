/** @file drawabletimesignature.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */
 
#ifndef DRAWABLETIMESIGNATURE_H_
#define DRAWABLETIMESIGNATURE_H_

#include "drawable/drawablemuselement.h"

class CATimeSignature;
class CADrawableStaff;

class CADrawableTimeSignature : public CADrawableMusElement {
	public:
		CADrawableTimeSignature(CATimeSignature *timeSig, CADrawableStaff *drawableStaff, int x, int y); /// y coordinate is a top of the staff
		~CADrawableTimeSignature();
		void draw(QPainter *p, CADrawSettings s);
		CADrawableTimeSignature *clone();
		inline CATimeSignature *timeSignature() { return (CATimeSignature*)_musElement; }
};

#endif /*DRAWABLETIMESIGNATURE_H_*/
