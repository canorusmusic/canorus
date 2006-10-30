/** @file drawable/drawable.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef DRAWABLE_H_
#define DRAWABLE_H_

#include <QRect>
#include <QColor>

class QPainter;

typedef struct CADrawSettings {
		float z;
		int x;
		int y;
		int w;
		int h;
		//QPenStyle penStyle;
		QColor color;
};

class CADrawable {
	public:
		CADrawable(int x, int y);	///x and y position of an element in absolute world units
		virtual void draw(QPainter *p, const CADrawSettings s) = 0;
		int xPos() { return _xPos + _xPosOffset; }
		int yPos() { return _yPos + _yPosOffset; }
		int xPosOrig() { return _xPos; }
		int yPosOrig() { return _yPos; }
		int width() { return _width; }
		int height() { return _height; }
		int neededWidth() { return _neededWidth; }
		int neededHeight() { return _neededHeight; }
		int xCenter() { return _xPos + _xPosOffset + (_width)/2; }
		int yCenter() { return _yPos + _yPosOffset + (_height)/2; }
		const QRect bBox() { return QRect(_xPos + _xPosOffset, _yPos + _yPosOffset, _width, _height); }
		bool isVisible() { return _visible; }
		bool isSelectable() { return _selectable; }
		
		/**
		 * Set X position of the drawable element.
		 * Note that these coordinates represent the pivot/center of the drawable element and the actual start of X can be somewhere else, depending on each element.
		 * By default, they set the xPos directly. By reimplementing the method in children class, its behaviour changes.
		 * 
		 * @param xPos X coordinate of the music element. This coordinate can finally differ from the given one, depending on the element type.
		 */
		virtual void setXPos(int xPos) { _xPos = xPos; }
		
		/**
		 * Set Y position of the drawable element.
		 * Note that these coordinates represent the pivot/center of the drawable element and the actual start of Y can be somewhere else, depending on each element.
		 * By default, they set the xPos directly. By reimplementing the method in children class, its behaviour changes.
		 * 
		 * eg. Y coordinate for CANote represents the center of the notehead, not the top edge of it.
		 * 
		 * @param yPos Y coordinate of the music element. This coordinate can finally differ from the given one, depending on the element type.
		 */
		virtual void setYPos(int yPos) { _yPos = yPos; }
		
		/**
		 * This method does the same as setXPos(), but sets the coordinates directly, without subclass transformations.
		 * 
		 * @param xPos X coordinate of the left edge of the music element.
		 */
		void setXPosAbsolute(int xPos) { _xPos = xPos; }
		
		/**
		 * This method does the same as setYPos(), but sets the coordinates directly, without subclass transformations.
		 * 
		 * @param yPos Y coordinate of the left edge of the music element.
		 */
		void setYPosAbsolute(int yPos) { _yPos = yPos; }
		void setXPosOffset(int xPosOffset) { _xPosOffset = xPosOffset; }
		void setYPosOffset(int yPosOffset) { _yPosOffset = yPosOffset; }
		void setWidth(int width) { _width = width; }
		void setHeight(int height) { _height = height; }
		void setVisible(bool v) { _visible = v; }
		
		virtual CADrawable *clone() = 0;
		
	protected:
		int _xPos;
		int _yPos;
		int _xPosOffset;
		int _yPosOffset;
		int _width;
		int _height;
		int _neededWidth;
		int _neededHeight;
		bool _visible;
		bool _selectable;	///Can the element be clicked on and is then selected
};
#endif /*DRAWABLE_H_*/

