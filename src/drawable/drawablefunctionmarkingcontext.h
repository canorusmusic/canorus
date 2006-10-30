/** @file drawable/drawablefunctionmarkingcontext.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef DRAWABLEFUNCTIONMARKINGCONTEXT_H_
#define DRAWABLEFUNCTIONMARKINGCONTEXT_H_

#include "drawable/drawablecontext.h"

class CAFunctionMarkingContext;

class CADrawableFunctionMarkingContext : public CADrawableContext {
	public:
		CADrawableFunctionMarkingContext(CAFunctionMarkingContext *c, int x, int y);
		~CADrawableFunctionMarkingContext();
		
		void draw(QPainter *p, const CADrawSettings s);
		CADrawableFunctionMarkingContext *clone();
};

#endif /*DRAWABLEFUNCTIONMARKINGCONTEXT_H_*/
