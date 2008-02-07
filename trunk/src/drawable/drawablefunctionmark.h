/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef DRAWABLEFUNCTIONMARK_H_
#define DRAWABLEFUNCTIONMARK_H_

#include "drawable/drawablemuselement.h"
#include "core/functionmark.h"

class CAFunctionMark;
class CADrawableFunctionMarkContext;

class CADrawableFunctionMark : public CADrawableMusElement {
public:
	CADrawableFunctionMark(CAFunctionMark *function, CADrawableFunctionMarkContext *context, int x, int y);
	~CADrawableFunctionMark();
	
	void draw(QPainter *p, const CADrawSettings s);
	CADrawableFunctionMark *clone(CADrawableContext* newContext = 0);
	
	inline CAFunctionMark *functionMark() { return (CAFunctionMark*)_musElement; };
	inline CADrawableFunctionMarkContext *drawableFunctionMarkContext() { return (CADrawableFunctionMarkContext*)_drawableContext; };
	
	bool isExtenderLineVisible() { return _extenderLineVisible; }
	void setExtenderLineVisible(bool visible) { _extenderLineVisible = visible; }
	bool isExtenderLineOnly() { return _extenderLineOnly; }
	void setExtenderLineOnly( bool line ) { _extenderLineOnly = line; }

private:		
	bool _extenderLineVisible; // Should the function draw a horizontal line until the end of the function		
	QString _text;             // Function transformed to String which is rendered then
	bool _extenderLineOnly;    // Only extender line should be rendered over the whole width of the function
};

class CADrawableFunctionMarkSupport : public CADrawableMusElement {
public:
	enum CADrawableFunctionMarkSupportType {
		Key,
		Rectangle,
		ChordArea,
		Tonicization,
		Ellipse,
		Alterations
	};
	// Key constructor
	CADrawableFunctionMarkSupport(CADrawableFunctionMarkSupportType, const QString key, CADrawableContext *c, int x, int y);
	// Rectangle, ChordArea, Tonicization, Ellipse constructor
	CADrawableFunctionMarkSupport(CADrawableFunctionMarkSupportType, CADrawableFunctionMark *function, CADrawableContext *c, int x, int y, CADrawableFunctionMark *function2=0);
	// Alterations consructor
	CADrawableFunctionMarkSupport(CADrawableFunctionMarkSupportType, CAFunctionMark *function, CADrawableContext *c, int x, int y);
	
	~CADrawableFunctionMarkSupport();
	
	void draw(QPainter *p, const CADrawSettings s);
	CADrawableFunctionMarkSupport *clone(CADrawableContext* newContext = 0);
	CADrawableFunctionMarkSupportType drawableFunctionMarkSupportType() { return _drawableFunctionMarkSupportType; }
	
	bool isExtenderLineVisible() { return _extenderLineVisible; }
	void setExtenderLineVisible(bool visible) { _extenderLineVisible = visible; }
	bool rectWider() { return _rectWider; }
	void setRectWider(bool wider) { if (!_rectWider) { _rectWider = wider; _yPos -= 3; _height += 6; } }
	
private:
	CADrawableFunctionMarkSupportType _drawableFunctionMarkSupportType;
	QString _key;
	CADrawableFunctionMark *_function1, *_function2;	// Tonicization's start/end functions
	bool _extenderLineVisible;							// Extender line when tonicization used after
	bool _rectWider;									// Is rectangle wider in height. Default: false. Useful when doing a series of modulations where you don't know which rectangle belongs to which function. Every 2nd rectangle is then a bit higher than the others.
};

#endif /* DRAWABLEFUNCTIONMARK_H_ */
