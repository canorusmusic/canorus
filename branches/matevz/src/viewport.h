#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QWidget>
#include <QPaintEvent>

#include "kdtree.h"

class CAViewPort : public QWidget {
	Q_OBJECT
	
	public:
		CAViewPort(QWidget *, CAKDTree *);
		void setWorldCoords(int x, int y, int w, int h);
		void setWorldCoords(QRect *r) {setWorldCoords(r->x(),r->y(),r->width(),r->height());}
		void setWorldCoords(int x, int y);
		void setCenterCoords(int x, int y);
		void setZoom(float z, int x=0, int y=0);

		inline void setWorldX(int x) {oldWorldX_ = worldX_; worldX_ = x;}
		inline void setWorldY(int y) {oldWorldY_ = worldY_; worldY_ = y;}
		inline void setWorldWidth(int w) {oldWorldW_ = worldW_; worldW_ = w;}
		inline void setWorldHeight(int h) {oldWorldH_ = worldH_; worldH_ = h;}

		const int worldX() {return worldX_;}
		const int worldY() {return worldY_;}
		const int worldWidth() {return worldW_;}
		const int worldHeight() {return worldH_;}
		const float zoom() {return zoom_;}
	
	signals:
		void mousePressEvent(QMouseEvent *);
		void wheelEvent(QWheelEvent *);
		
	private:
		void resizeEvent(QResizeEvent *);
		CAKDTree *musElements_;
		QWidget *parent_;
		int worldX_, worldY_, worldW_, worldH_;
		int oldWorldX_, oldWorldY_, oldWorldW_, oldWorldH_;		
		float zoom_;
		void paintEvent(QPaintEvent *);
};
#endif
