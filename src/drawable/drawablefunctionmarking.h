/** @file drawable/drawablefunctionmarking.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef DRAWABLEFUNCTIONMARKING_H_
#define DRAWABLEFUNCTIONMARKING_H_

#include "drawable/drawablemuselement.h"

class CAFunctionMarking;
class CADrawableFunctionMarkingContext;

class CADrawableFunctionMarking : public CADrawableMusElement {
	public:
		CADrawableFunctionMarking(CAFunctionMarking *function, CADrawableFunctionMarkingContext *context, int x, int y);
		~CADrawableFunctionMarking();
		
		void draw(QPainter *p, const CADrawSettings s);
		CADrawableFunctionMarking *clone();
		
		inline CAFunctionMarking *functionMarking() { return (CAFunctionMarking*)_musElement; };
		inline CADrawableFunctionMarkingContext *drawableFunctionMarkingContext() { return (CADrawableFunctionMarkingContext*)_drawableContext; };
		
		void setExtenderLineVisible(bool visible) { _extenderLine = visible; }
		bool isExtenderLineVisible() { return _extenderLine; }
	
	private:
		bool _extenderLine;		///Should the function draw a horizontal line until the end of the function
};

#endif /*DRAWABLEFUNCTIONMARKING_H_*/
