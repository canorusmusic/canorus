/*! 
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef DRAWABLE_H_
#define DRAWABLE_H_

#include <QRect>
#include <QColor>

class QPainter;

struct CADrawSettings {
		float z;
		int x;
		int y;
		int w;
		int h;
		// QPenStyle penStyle;
		QColor color;
};

class CADrawable {
public:
	enum CADrawableType {
		DrawableMusElement,
		DrawableContext
	};
	
	CADrawable(int x, int y);	// x and y position of an element in absolute world units	
	virtual ~CADrawable() { }
	virtual void draw(QPainter *p, const CADrawSettings s) = 0;
	virtual CADrawable *clone();
	
	void drawHScaleHandles( QPainter *p, const CADrawSettings s );
	void drawVScaleHandles( QPainter *p, const CADrawSettings s );
	
	inline CADrawableType drawableType() { return _drawableType; }
	inline int xPos() { return _xPos + _xPosOffset; }
	inline int yPos() { return _yPos + _yPosOffset; }
	inline int xPosOrig() { return _xPos; }
	inline int yPosOrig() { return _yPos; }
	inline int width() { return _width; }
	inline int height() { return _height; }
	inline int neededWidth() { return _neededWidth; }
	inline int neededHeight() { return _neededHeight; }
	inline int xCenter() { return _xPos + _xPosOffset + (_width)/2; }
	inline int yCenter() { return _yPos + _yPosOffset + (_height)/2; }
	inline const QRect bBox() { return QRect(_xPos + _xPosOffset, _yPos + _yPosOffset, _width, _height); }
	inline bool isVisible() { return _visible; }
	inline bool isSelectable() { return _selectable; }
	inline bool isHScalable() { return _HScalable; }
	inline bool isVScalable() { return _VScalable; }
	
	inline virtual void setXPos(int xPos) { _xPos = xPos; }
	inline virtual void setYPos(int yPos) { _yPos = yPos; }
	inline void setXPosAbsolute(int xPos) { _xPos = xPos; } // set the xPos directly without any specific offsets
	inline void setYPosAbsolute(int yPos) { _yPos = yPos; } // set the yPos directly without any specific offsets
	inline void setXPosOffset(int xPosOffset) { _xPosOffset = xPosOffset; }
	inline void setYPosOffset(int yPosOffset) { _yPosOffset = yPosOffset; }
	inline void setWidth(int width) { _neededWidth += (width - _width); _width = width; }
	inline void setHeight(int height) { _neededHeight += (height - _height); _height = height; }
	inline void setVisible(bool v) { _visible = v; }
	inline void setSelectable(bool s) { _selectable = s; }
	inline void setHScalable(bool s) { _HScalable = s; }
	inline void setVScalable(bool s) { _VScalable = s; }
	
protected:
	inline void setNeededWidth(int width) { _neededWidth = width; }
	inline void setNeededHeight(int height) { _neededHeight = height; }
	CADrawableType _drawableType; // DrawableMusElement or DrawableContext.
	int _xPos;
	int _yPos;
	int _xPosOffset;
	int _yPosOffset;
	int _width;        // Element's width as it appears on the screen
	int _height;       // Element's height as it appears on the screen
	int _neededWidth;  // Minimum width the next element should be placed next to it by engraver
	int _neededHeight; // Minimum height the next element should be placed next to it by engraver
	bool _visible;
	bool _selectable;	// Can the element be clicked on and is then selected
	bool _HScalable;    // Can the element be streched horizontally
	bool _VScalable;    // Can the element be streched vertically
	
	static const int SCALE_HANDLES_SIZE; // Width and Height of the scale handles squares
};

#endif /* DRAWABLE_H_ */
