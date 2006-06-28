#include "viewport.h"
#include <iostream>

CAViewPort::CAViewPort(QWidget *p, CAKDTree *t) : QWidget(p) {
	parent_ = p;
	worldX_ = worldY_ = 0;
	zoom_ = 1.0;
	musElements_ = t;
}

void CAViewPort::setWorldCoords(int x, int y, int w, int h) {
	setWorldX(x);
	setWorldY(y);
	setWorldWidth(w);
	setWorldHeight(h);

	zoom_ = width() / w;
	
	repaint();
}

void CAViewPort::setWorldCoords(int x, int y) {
	setWorldX(x);
	setWorldY(y);
	
	repaint();
}

void CAViewPort::setCenterCoords(int x, int y) {
	int t;
	if (t = x - (int)(0.5*worldW_) < 0) t = 0;
	setWorldX(t);
	if (t = y - (int)(0.5*worldH_) < 0) t = 0;
	setWorldY(t);
	
	repaint();
}

void CAViewPort::setZoom(float z, int x, int y) {
	setWorldWidth((int)(width() / z));
	setWorldHeight((int)(height() / z));

	bool zoomOut = false;
	if (zoom_ - z > 0)
		zoomOut = true;
	
	zoom_ = z;
	
	int t;
	if (!zoomOut) {
		t = (int)(x/(z*2)) + worldX_ - (int)(0.25*worldW_);
		setWorldX(t>0?t:0);
		t = (int)(y/(z*2)) + worldY_ - (int)(0.25*worldH_);
		setWorldY(t>0?t:0);
	} else {
		t = worldX_ - (int)(0.5*(worldW_ - oldWorldW_));
		setWorldX(t>0?t:0);
		t = worldY_ - (int)(0.5*(worldH_ - oldWorldH_));
		setWorldY(t>0?t:0);
	}
	
	repaint();
}

void CAViewPort::paintEvent(QPaintEvent *e) {
	if (!musElements_)
		return;
		
	QPainter p(this);
	QList<CADrawable *>* l = musElements_->findInRange(worldX_, worldY_, worldW_, worldH_);
	
	for (int i=0; i<l->size(); i++) {
		l->at(i)->draw(&p,
		               (int)((l->at(i)->xPos() - worldX_) * zoom_),
		               (int)((l->at(i)->yPos() - worldY_) * zoom_),
		               zoom_);
	}
	
	//flush the oldWorld coordinates as they're needed for the first repaint only
	oldWorldX_ = worldX_; oldWorldY_ = worldY_;
	oldWorldW_ = worldW_; oldWorldH_ = worldH_;	
}

void CAViewPort::resizeEvent(QResizeEvent *e) {
	oldWorldW_ = worldW_;
	oldWorldH_ = worldH_;
	worldW_ = (int)(width() / zoom_);
	worldH_ = (int)(height() / zoom_);
	
	repaint();
}
