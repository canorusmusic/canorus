/** @file drawable.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef DRAWABLE_H_
#define DRAWABLE_H_

#include <QRect>

class QPainter;

class CADrawable {
	public:
		CADrawable();
		virtual void draw(QPainter *, int x, int y, float z) = 0;
		int xPos() { return _xPos + _xPosOffset; }
		int yPos() { return _yPos + _yPosOffset; }
		int width() { return _width; }
		int height() { return _height; }
		int xCenter() { return _xPos + _xPosOffset + (_width)/2; }
		int yCenter() { return _yPos + _yPosOffset + (_height)/2; }
		const QRect bBox() { return QRect(_xPos + _xPosOffset, _yPos + _yPosOffset, _width, _height); }
		bool visible() { return _visible; }
		
		void setXPos(int xPos) { _xPos = xPos; }
		void setYPos(int yPos) { _yPos = yPos; }
		void setXPosOffset(int xPosOffset) { _xPosOffset = xPosOffset; }
		void setYPosOffset(int yPosOffset) { _yPosOffset = yPosOffset; }
		void setWidth(int width) { _width = width; }
		void setHeight(int height) { _height = height; }
		void setVisible(bool v) { _visible = v; }

	protected:
		int _xPos;
		int _yPos;
		int _xPosOffset;
		int _yPosOffset;
		int _width;
		int _height;
		bool _visible;
};
#endif /*DRAWABLE_H_*/

