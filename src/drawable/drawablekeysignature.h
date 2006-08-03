/** @file drawablekeysignature.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef DRAWABLEKEYSIGNATURE_H_
#define DRAWABLEKEYSIGNATURE_H_

#include "drawable/drawablemuselement.h"

class CADrawableStaff;
class CAKeySignature;
class CADrawableAccidental;

class CADrawableKeySignature : public CADrawableMusElement {
	public:
		/**
		 * Default constructor.
		 * 
		 * @param y Marks the top line Y coordinate of the staff in absolute world units.
		 */ 
		CADrawableKeySignature(CAKeySignature *keySig, CADrawableStaff *staff, int x, int y);
		~CADrawableKeySignature();
		
		void draw(QPainter *p, CADrawSettings s);
		CADrawableKeySignature *clone();
		inline CAKeySignature *keySignature() { return (CAKeySignature*)_musElement; }
	
	private:
		QList<CADrawableAccidental*> _drawableAccidentalList;	///List of actual drawable accidentals
};

#endif /*DRAWABLEKEYSIGNATURE_H_*/
