#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <QPainter>
#include <QRect>

class CADrawable {
	public:
		CADrawable();
		virtual void draw(QPainter *, int x, int y, float z) = 0;
		int xPos() {return xPos_ + xPosOffset_;}
		int yPos() {return yPos_ + yPosOffset_;}
		int width() {return width_;}
		int height() {return height_;}
		int xCenter() {return xPos_ + xPosOffset_ + (width_)/2;}
		int yCenter() {return yPos_ + yPosOffset_ + (height_)/2;}
		const QRect bBox() {return QRect(xPos_ + xPosOffset_, yPos_ + yPosOffset_, width_, height_);}
		bool visible() {return visible_;}
		
		void setVisible(bool v) {visible_ = v;}

	protected:
		int xPos_;
		int yPos_;
		int xPosOffset_;
		int yPosOffset_;
		int width_;
		int height_;
		bool visible_;
};
#endif

