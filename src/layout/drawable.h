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
		float z;       // zoom level
		int x;         // painter x pos in pixels
		int y;         // painter y pos in pixels
		int w;         // canvas width in pixels
		int h;         // canvas height in pixels
		QColor color;  // pen color
		double worldX; // x coordinate of the view
		double worldY; // y coordinate of the view
};

class CADrawable {
public:
	enum CADrawableType {
		DrawableMusElement,
		DrawableContext
	};

	enum CADirection {
		Undefined,
		Top,
		Bottom,
		Left,
		Right,
		TopLeft,
		TopRight,
		BottomLeft,
		BottomRight
	};

	CADrawable( double x, double y );	// x and y position of an element in absolute world units
	virtual ~CADrawable() { }
	virtual void draw(QPainter *p, const CADrawSettings s) = 0;
	virtual CADrawable *clone();

	void drawHScaleHandles( QPainter *p, const CADrawSettings s );
	void drawVScaleHandles( QPainter *p, const CADrawSettings s );

	inline CADrawableType drawableType() { return _drawableType; }
	inline double xPos() const { return _xPos; }
	inline double yPos() const { return _yPos; }
	inline double width() const { return _width; }
	inline double height() const { return _height; }
	inline double neededSpaceWidth() const { return _neededSpaceWidth; }
	inline double neededSpaceHeight() const { return _neededSpaceHeight; }
	inline double neededWidth() const { return _width+_neededSpaceWidth; }
	inline double neededHeight() const { return _height+_neededSpaceHeight; }
	inline double xCenter() const { return _xPos + (_width)/2; }
	inline double yCenter() const { return _yPos + (_height)/2; }
	inline const QRect bBox() const { return QRect(_xPos, _yPos, _width, _height); }
	inline bool isVisible() const { return _visible; }
	inline bool isSelectable() const { return _selectable; }
	inline bool isHScalable() const { return _hScalable; }
	inline bool isVScalable() const { return _vScalable; }

	inline void setXPos(double xPos) { _xPos = xPos; }
	inline void setYPos(double yPos) { _yPos = yPos; }
	inline void setWidth(double width) { _width = width; }
	inline void setHeight(double height) { _height = height; }
	inline void setNeededSpaceWidth( double width ) { _neededSpaceWidth = width; }
	inline void setNeededSpaceHeight( double height ) { _neededSpaceHeight = height; }
	inline void setVisible(bool v) { _visible = v; }
	inline void setSelectable(bool s) { _selectable = s; }
	inline void setHScalable(bool s) { _hScalable = s; }
	inline void setVScalable(bool s) { _vScalable = s; }

protected:
	void setDrawableType( CADrawableType t ) { _drawableType = t; };

	CADrawableType _drawableType; // DrawableMusElement or DrawableContext.
	double _xPos;
	double _yPos;
	double _width;             // Element's width as it appears on the screen
	double _height;            // Element's height as it appears on the screen
	double _neededSpaceWidth;  // Minimum width the next element should be placed next to it by engraver
	double _neededSpaceHeight; // Minimum height the next element should be placed next to it by engraver
	bool _visible;
	bool _selectable;	// Can the element be clicked on and is then selected
	bool _hScalable;    // Can the element be streched horizontally
	bool _vScalable;    // Can the element be streched vertically

	static const int SCALE_HANDLES_SIZE; // Width and Height of the scale handles squares in pixels
};

#endif /* DRAWABLE_H_ */
