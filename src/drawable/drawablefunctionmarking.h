/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef DRAWABLEFUNCTIONMARKING_H_
#define DRAWABLEFUNCTIONMARKING_H_

#include "drawable/drawablemuselement.h"
#include "core/functionmarking.h"

class CAFunctionMarking;
class CADrawableFunctionMarkingContext;

/*!
	\class CADrawableFunctionMarking
	\brief The drawable instance of the function marking.
	These music elements are ordinary music elements selectable by the user.
*/
class CADrawableFunctionMarking : public CADrawableMusElement {
	public:
		CADrawableFunctionMarking(CAFunctionMarking *function, CADrawableFunctionMarkingContext *context, int x, int y);
		~CADrawableFunctionMarking();
		
		void draw(QPainter *p, const CADrawSettings s);
		CADrawableFunctionMarking *clone(CADrawableContext* newContext = 0);
		
		inline CAFunctionMarking *functionMarking() { return (CAFunctionMarking*)_musElement; };
		inline CADrawableFunctionMarkingContext *drawableFunctionMarkingContext() { return (CADrawableFunctionMarkingContext*)_drawableContext; };
		
		void setExtenderLineVisible(bool visible) { _extenderLineVisible = visible; }
		bool isExtenderLineVisible() { return _extenderLineVisible; }
		bool isExtenderLineOnly() { return _extenderLineOnly; }
		void setExtenderLineOnly( bool line ) { _extenderLineOnly = line; }
	
	private:		
		bool _extenderLineVisible; // Should the function draw a horizontal line until the end of the function		
		QString _text;             // Function transformed to String which is rendered then
		bool _extenderLineOnly;    // Only extender line should be rendered over the whole width of the function
};

/*!
	Support class which draws the key of the function, rectangle around it, chord area, ellipse etc.
	These drawable music elements aren't selectable, but they can't be drawn by a single CADrawableFunctionMarking because they're usually dependent on more than one function marking. 
*/
class CADrawableFunctionMarkingSupport : public CADrawableMusElement {
	public:
		enum CADrawableFunctionMarkingSupportType {
			Key,
			Rectangle,
			ChordArea,
			Tonicization,
			Ellipse,
			Alterations
		};
		// Key constructor
		CADrawableFunctionMarkingSupport(CADrawableFunctionMarkingSupportType, const QString key, CADrawableContext *c, int x, int y);
		// Rectangle, ChordArea, Tonicization, Ellipse constructor
		CADrawableFunctionMarkingSupport(CADrawableFunctionMarkingSupportType, CADrawableFunctionMarking *function, CADrawableContext *c, int x, int y, CADrawableFunctionMarking *function2=0);
		// Alterations consructor
		CADrawableFunctionMarkingSupport(CADrawableFunctionMarkingSupportType, CAFunctionMarking *function, CADrawableContext *c, int x, int y);
		
		~CADrawableFunctionMarkingSupport();
		
		void draw(QPainter *p, const CADrawSettings s);
		CADrawableFunctionMarkingSupport *clone(CADrawableContext* newContext = 0);
		CADrawableFunctionMarkingSupportType drawableFunctionMarkingSupportType() { return _drawableFunctionMarkingSupportType; }
		
		bool isExtenderLineVisible() { return _extenderLineVisible; }
		void setExtenderLineVisible(bool visible) { _extenderLineVisible = visible; }
		bool rectWider() { return _rectWider; }
		void setRectWider(bool wider) { if (!_rectWider) { _rectWider = wider; _yPos -= 3; _height += 6; } }
	
	private:
		CADrawableFunctionMarkingSupportType _drawableFunctionMarkingSupportType;
		QString _key;
		CADrawableFunctionMarking *_function1, *_function2;	// Tonicization's start/end functions
		bool _extenderLineVisible;							// Extender line when tonicization used after
		bool _rectWider;									// Is rectangle wider in height. Default: false. Useful when doing a series of modulations where you don't know which rectangle belongs to which function. Every 2nd rectangle is then a bit higher than the others.
};

#endif /* DRAWABLEFUNCTIONMARKING_H_ */
