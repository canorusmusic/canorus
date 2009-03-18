/*!
	Copyright (c) 2006-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef DRAWABLETIMESIGNATURE_H_
#define DRAWABLETIMESIGNATURE_H_

#include "drawable/drawablemuselement.h"
#include "core/timesignature.h"
#include "canorus.h"

class CADrawableStaff;

class CADrawableTimeSignature : public CADrawableMusElement {
	public:
		CADrawableTimeSignature(CATimeSignature *timeSig, CADrawableStaff *drawableStaff, double x, double y); /// y coordinate is a top of the staff
		~CADrawableTimeSignature();
		void draw(QPainter *p, CADrawSettings s);
		CADrawableTimeSignature *clone(CADrawableContext* newContext = 0);
		inline CATimeSignature *timeSignature() { return static_cast<CATimeSignature*>(_musElement); }
};

#endif /*DRAWABLETIMESIGNATURE_H_*/
