#include <QScrollBar>
#include <iostream>
#include "viewport.h"

CAViewPort::CAViewPort(QWidget *p, CAKDTree *t) : QWidget(p) {
	parent_ = p;
	worldX_ = worldY_ = 0;
	zoom_ = 1.0;
	musElements_ = t;
	holdRepaint_ = false;
	
	//setup the mouse events and forward them to CAEventName events
	connect(this, SIGNAL(mousePressEvent(QMouseEvent *)), this, SLOT(processMousePressEvent(QMouseEvent *)));
	connect(this, SIGNAL(wheelEvent(QWheelEvent *)), this, SLOT(processWheelEvent(QWheelEvent *)));

	//setup the virtual canvas
	canvas_ = new QWidget(this);

	//setup the scrollbars
	vScrollBar_ = new QScrollBar(Qt::Vertical, this);
	hScrollBar_ = new QScrollBar(Qt::Horizontal, this);
	vScrollBar_->setMinimum(0);
	hScrollBar_->setMinimum(0);
	vScrollBar_->setTracking(true); //trigger valueChanged() when dragging the slider, not only releasing it
	hScrollBar_->setTracking(true);
	vScrollBar_->hide();
	hScrollBar_->hide();
	scrollBarsVisible_ = ScrollBarShowIfNeeded;
	allowManualScroll_ = true;
	
	connect(hScrollBar_, SIGNAL(valueChanged(int)), this, SLOT(processHScrollBarEvent(int)));
	connect(vScrollBar_, SIGNAL(valueChanged(int)), this, SLOT(processVScrollBarEvent(int)));
	
	//setup layout
	layout_ = new QGridLayout(this);
	layout_->setMargin(0);
	layout_->addWidget(canvas_, 0, 0);
	layout_->addWidget(vScrollBar_, 0, 1);
	layout_->addWidget(hScrollBar_, 1, 0);
	this->setLayout(layout_);
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAViewPort::setWorldX(int x, bool force) {
	if (!force || !musElements_) {
		int maxX = musElements_->getMaxX();
		if (x > maxX - worldW_)
			x = maxX - worldW_;
		if (x < 0)
			x = 0;
	}
	
	oldWorldX_ = worldX_;
	worldX_ = x;
	holdRepaint_ = true;
	hScrollBar_->setValue(x);
	holdRepaint_ = false;

	checkScrollBars();
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAViewPort::setWorldY(int y, bool force) {
	if (!force || !musElements_) {
		int maxY = musElements_->getMaxY();
		if (y > maxY - worldH_)
			y = maxY - worldH_;
		if (y < 0)
			y = 0;
	}

	oldWorldY_ = worldY_;
	worldY_ = y;
	holdRepaint_ = true;
	vScrollBar_->setValue(y);
	holdRepaint_ = false;
	
	checkScrollBars();
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAViewPort::setWorldWidth(int w, bool force) {
	if (!force) {
		if (w < 1)
			w = 1;
	}
	
	oldWorldW_ = worldW_;
	worldW_ = w;
	
	int scrollMax;
	if ((musElements_) && (scrollMax = musElements_->getMaxX() - worldW_ > 0));
		hScrollBar_->setMaximum(scrollMax);
	
	zoom_ = ((float)drawableWidth() / worldW_);

	checkScrollBars();
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAViewPort::setWorldHeight(int h, bool force) {
	if (!force) {
		if (h < 1)
			h = 1;
	}
	
	oldWorldH_ = worldH_;
	worldH_ = h;

	int scrollMax;
	if ((musElements_) && (scrollMax = musElements_->getMaxY() - worldH_ > 0));
		vScrollBar_->setMaximum(scrollMax);

	zoom_ = ((float)drawableHeight() / worldH_);

	checkScrollBars();
}

/**
 * Set the viewport virtual rectangle dimensions.
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 * 
 * @param int x X coordinate of top-left corner in world units
 * @param int y Y coordinate of top-left corner in world units
 * @param int w Width of the rectangle
 * @param int h Height of the rectangle
 * @param bool force Ignore if the given coordinates are illegal (like negative numbers, >maximum width etc.)
 */
void CAViewPort::setWorldCoords(int x, int y, int w, int h, bool force) {
	setWorldX(x, force);
	setWorldY(y, force);
	setWorldWidth(w, force);
	setWorldHeight(h, force);
}

/**
 * Set the top-left coordinates of the viewport rectangle.
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 * 
 * @param int x X coordinate in world units
 * @param int y Y coordinate in world units
 * @param bool force Ignore if the given coordinates are illegal (like negative numbers, >maximum width etc.)
 */
void CAViewPort::setWorldCoords(int x, int y, bool force) {
	setWorldX(x, force);
	setWorldY(y, force);
}

/**
 * Pan to the coordinates, so the given coordinates represent the center of the widget.
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 * 
 * @param int x X Coordinate in world units
 * @param int y Y Coordinate in world units
 * @param bool force Ignore if the given coordinates are illegal (like negative numbers, >maximum width etc.)
 */
void CAViewPort::setCenterCoords(int x, int y, bool force) {
	setWorldX(x - (int)(0.5*worldW_), force);
	setWorldY(y - (int)(0.5*worldH_), force);
}

/**
 * Set zoom to the given factor.
 * If zooming in, the given coordintes give the direction where to zoom in.
 * If zooming out, the coordinates direction is ignored.
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 * 
 * @param float z Zoom factor (1.0 = 100%, 1.5 = 150%)
 * @param int x Zoom direction in absolute world units
 * @param int y Zoom direction in absolute world units
 * @param bool force Ignore if the given coordinates are illegal (like negative numbers, >maximum width etc.)
 */
void CAViewPort::setZoom(float z, int x, int y, bool force) {
	bool zoomOut = false;
	if (zoom_ - z > 0.0)
		zoomOut = true;

	//set the world width - updates the zoom level zoom_ as well
	setWorldWidth((int)(drawableWidth() / z));
	setWorldHeight((int)(drawableHeight() / z));
	
	if (!zoomOut) { //zoom in
		//the new view's center coordinates will become the middle point of the current viewport center coords and the mouse pointer coords
		setCenterCoords( ( worldX_ + (worldW_/2) + x ) / 2,
		                 ( worldY_ + (worldH_/2) + y ) / 2,
		                 force );
	} else { //zoom out
		//the new view's center coordinates will become the middle point of the current viewport center coords and the mirrored over center pointer coords
		//worldX_ + (worldW_/2) + (worldX_ + (worldW_/2) - x)/2
		setCenterCoords( (int)(1.5*worldX_ + 0.75*worldW_ - 0.5*x),
		                 (int)(1.5*worldY_ + 0.75*worldH_ - 0.5*y),
		                 force );
	}
	
	checkScrollBars();
}

void CAViewPort::paintEvent(QPaintEvent *e) {
	if (!musElements_ || holdRepaint_)
		return;
	
	QPainter p(this);
	p.drawLine(0, 0, drawableWidth(), drawableHeight());
	QList<CADrawable *>* l = musElements_->findInRange(worldX_, worldY_, worldW_, worldH_);
	if (!l) return;

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

/**
 * Called on resize event, like user resizes the main window etc.
 * The virtual canvas widget's resize event is connected to this method as well.
 * 
 * @param QResizeEvent *e Qt resize event
 */
void CAViewPort::resizeEvent(QResizeEvent *e) {
	oldWorldW_ = worldW_;
	oldWorldH_ = worldH_;
	worldW_ = (int)(drawableWidth() / zoom_);
	worldH_ = (int)(drawableHeight() / zoom_);
	
	checkScrollBars();

	repaint();
}

/**
 * Show/hide scrollbars, if needed.
 * If scrollBarsVisible_ is set to AlwaysVisible or AlwaysHidden, this method does nothing.
 * If set to ShowIfNeeded, check if the whole world scene can get rendered to the widget and
 * then show the scrollbars, if can't and hide them, if the scene can be rendered.
 * This function calls repaint automatically upon virtual canvas resize event. Otherwise, it doesn't.
 */
void CAViewPort::checkScrollBars() {
	if ((scrollBarsVisible_ != ScrollBarShowIfNeeded) || (!musElements_))
		return;

	bool change = false;
	holdRepaint_ = true;
	if ((musElements_->getMaxX() - worldWidth() > 0) || (hScrollBar_->value()!=0)) { //if scrollbar is needed
		if (!hScrollBar_->isVisible()) {
			hScrollBar_->show();
			setWorldHeight((int)(drawableHeight() / zoom_));
			change = true;
		}
	} else //if the whole scene can be drawn on the canvas and the scrollbars are at position 0
		if (hScrollBar_->isVisible()) {
			hScrollBar_->hide();
			setWorldHeight((int)(drawableHeight() / zoom_));
			change = true;
		}
		
	if ((musElements_->getMaxY() - worldHeight() > 0) || (vScrollBar_->value()!=0)) { //if scrollbar is needed
		if (!vScrollBar_->isVisible()) {
			vScrollBar_->show();
			setWorldWidth((int)(drawableWidth() / zoom_));
			change = true;
		}
	} else //if the whole scene can be drawn on the canvas and the scrollbars are at position 0
		if (vScrollBar_->isVisible()) {
			vScrollBar_->hide();
			setWorldWidth((int)(drawableWidth() / zoom_));
			change = true;
		}
	
	holdRepaint_ = false;
	if (change)
		repaint();
}

void CAViewPort::processMousePressEvent(QMouseEvent *e) {
	emit CAMousePressEvent(e, QPoint((int)(e->x() / zoom_) + worldX_, (int)(e->y() / zoom_) + worldY_), this);
}

void CAViewPort::processWheelEvent(QWheelEvent *e) {
	emit CAWheelEvent(e, QPoint((int)(e->x() / zoom_) + worldX_, (int)(e->y() / zoom_) + worldY_), this);	
}

/**
 * Set the scrollbars visibility.
 * 
 * @param chat status CAViewPort::ScrollBarAlwaysVisible, ScrollBarAlwaysHidden, ScrollBarShowIfNeeded
 */
void CAViewPort::setScrollBarsVisibility(char status) {
	scrollBarsVisible_ = status;
	
	if (status==ScrollBarAlwaysVisible) {
		hScrollBar_->show();
		vScrollBar_->show();
		return;
	}
	
	if (status==ScrollBarAlwaysHidden) {
		hScrollBar_->hide();
		vScrollBar_->hide();
		return;
	}
	
	checkScrollBars();
}

void CAViewPort::processHScrollBarEvent(int val) {
	if (allowManualScroll_) {
		setWorldX(val);
		repaint();
	}
}

void CAViewPort::processVScrollBarEvent(int val) {
	if (allowManualScroll_) {
		setWorldY(val);
		repaint();
	}
}
