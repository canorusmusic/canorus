/** @file drawable/drawablerest.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef DRAWABLEREST_H_
#define DRAWABLEREST_H_

#include "drawable/drawablemuselement.h"

class CARest;

class CADrawableRest : public CADrawableMusElement {
	public:
		CADrawableRest(CARest *rest, CADrawableContext *drawableContext, int x, int y);
		CADrawableRest *clone();
		~CADrawableRest();
		
		void draw(QPainter *p, CADrawSettings s);
		
		inline CARest* rest() { return (CARest*)_musElement; }
};

#endif /*DRAWABLEREST_H_*/
