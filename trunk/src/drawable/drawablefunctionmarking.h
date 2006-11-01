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
		
		void setExtenderLineVisible(bool visible) { _extenderLineVisible = visible; }
		bool isExtenderLineVisible() { return _extenderLineVisible; }
	
	private:
		bool _extenderLineVisible;		///Should the function draw a horizontal line until the end of the function
		int _fontWidth;			///Width of the font itself - needed to determine where to start the extender line
};

/**
 * Support class which draws the key of the function, rectangle around it, chord area, ellipse etc.
 * These drawable music elements aren't selectable and can't actually belong to a single function. They're only there for aesthetic purposes, so a user can faster find himself. 
 */
class CADrawableFunctionMarkingSupport : public CADrawableMusElement {
	public:
		enum CADrawableFunctionMarkingSupportType {
			Key,
			Rectangle,
			ChordArea,
			TonicizationSingle,
			TonicizationDouble,
			Ellipse
		};
		//Key constructor
		CADrawableFunctionMarkingSupport(CADrawableFunctionMarkingSupportType, const QString key, CADrawableContext *c, int x, int y);
		~CADrawableFunctionMarkingSupport();
		
		void draw(QPainter *p, const CADrawSettings s);
		CADrawableFunctionMarkingSupport *clone();
	
	private:
		QString _key;
		CADrawableFunctionMarkingSupportType _drawableFunctionMarkingSupportType;
};

#endif /*DRAWABLEFUNCTIONMARKING_H_*/
