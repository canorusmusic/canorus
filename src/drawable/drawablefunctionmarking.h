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
#include "core/functionmarking.h"

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
		QString _text;			///Function transformed to String which is rendered then
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
			Tonicization,
			Ellipse
		};
		//Key constructor
		CADrawableFunctionMarkingSupport(CADrawableFunctionMarkingSupportType, const QString key, CADrawableContext *c, int x, int y);
		//Rectangle, ChordArea, Tonicization, Ellipse constructor		
		CADrawableFunctionMarkingSupport(CADrawableFunctionMarkingSupportType, CADrawableFunctionMarking *function, CADrawableContext *c, int x, int y, CADrawableFunctionMarking *function2=0);
		~CADrawableFunctionMarkingSupport();
		
		void draw(QPainter *p, const CADrawSettings s);
		CADrawableFunctionMarkingSupport *clone();
		CADrawableFunctionMarkingSupportType drawableFunctionMarkingSupportType() { return _drawableFunctionMarkingSupportType; }
		
		bool isExtenderLineVisible() { return _extenderLineVisible; }
		void setExtenderLineVisible(bool visible) { _extenderLineVisible = visible; }
		bool rectWider() { return _rectWider; }
		void setRectWider(bool wider) { if (!_rectWider) { _rectWider = wider; _yPos -= 3; _height += 6; } }
	
	private:
		CADrawableFunctionMarkingSupportType _drawableFunctionMarkingSupportType;
		QString _key;
		CADrawableFunctionMarking *_function1, *_function2;	//Tonicization's start/end functions
		bool _extenderLineVisible;							//Extender line when tonicization used
		bool _rectWider;									//Is rectangle wider in height. Default: false. Useful when doing a series of modulations where you don't know which rectangle belongs to which function. Every 2nd rectangle is then a bit higher than the others.
};

#endif /*DRAWABLEFUNCTIONMARKING_H_*/
