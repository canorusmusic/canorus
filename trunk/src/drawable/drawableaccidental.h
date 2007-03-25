/*!
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef DRAWABLEACCIDENTAL_H_
#define DRAWABLEACCIDENTAL_H_

#include "drawable/drawablemuselement.h"

class CADrawableAccidental : public CADrawableMusElement {
	public:
		/**
		 * Default constructor.
		 * 
		 * @param accs Type of the accidental: 0 - natural, -1 - flat, +1 - sharp, -2 doubleflat, +2 - cross etc.
		 * @param musElement Pointer to the according musElement which the accidental represents (usually a CANote or CAKeySignature).
		 * @param drawableContext Pointer to the according drawable context which the accidental belongs to (usually CADrawableStaff or CADrawableFiguredBass).
		 * @param x Left X-coordinate of the accidental.
		 * @param y Center Y-coordinate of the accidental.
		 */
		CADrawableAccidental(signed char accs, CAMusElement *musElement, CADrawableContext *drawableContext, int x, int y);
		~CADrawableAccidental();
		void draw(QPainter *p, CADrawSettings s);
		CADrawableAccidental *clone(CADrawableContext* newContext = 0);
	
	private:
		signed char _accs;
		int _centerX, _centerY;	//easier to do clone(), otherwise not needed
};

#endif /* DRAWABLEACCIDENTAL_H_ */
