/*!
	Copyright (c) 2006-2010, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef DRAWABLE_H_
#define DRAWABLE_H_

#include <QRect>
#include <QGraphicsItem>
#include <QColor>

class QPainter;
class CAMusElement;
class CAContext;

class CADrawable : public QGraphicsItemGroup {
public:
	enum CADrawableType {
		DrawableContext,
		DrawableMusElement
	};

	enum CADirection {
		Undefined=-1,
		Top,
		Bottom,
		Left,
		Right,
		TopLeft,
		TopRight,
		BottomLeft,
		BottomRight
	};

	CADrawable( const CADrawableType& drawableType );
	virtual ~CADrawable() { }
	virtual CADrawable *clone() { return 0; }

	virtual void setColor( QColor ) { } // used for selected items

	virtual void setWidth( double ) { }
	virtual void setHeight( double ) { }


/*	void drawHScaleHandles( QPainter *p, const CADrawSettings s );
	void drawVScaleHandles( QPainter *p, const CADrawSettings s );
*/
	inline CADrawableType& drawableType() { return _drawableType; }

	inline bool isHScalable() { return _hScalable; }
	inline bool isVScalable() { return _vScalable; }

	inline void setHScalable(bool s) { _hScalable = s; }
	inline void setVScalable(bool s) { _vScalable = s; }

protected:
	CADrawableType _drawableType; // DrawableMusElement or DrawableContext.
	bool _hScalable;              // Can the element be streched horizontally
	bool _vScalable;              // Can the element be streched vertically

	static const int SCALE_HANDLES_SIZE; // Width and Height of the scale handles squares in pixels

protected slots:
	void mousePressEvent( QGraphicsSceneMouseEvent * event );
};

#endif /* DRAWABLE_H_ */
