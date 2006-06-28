#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <iostream>


#include "scrollwidget.h"
#include "note.h"

using namespace std;

CAScrollWidget::CAScrollWidget(QWidget *parent) : QFrame(parent) {
	canvas_ = new CAViewPort(this, &musElements_);
	vScrollBar_ = new QScrollBar(Qt::Vertical, this);
	hScrollBar_ = new QScrollBar(Qt::Horizontal, this);
	vScrollBar_->setMinimum(0);
	hScrollBar_->setMinimum(0);
	vScrollBar_->hide();
	hScrollBar_->hide();	
	
	connect(vScrollBar_, SIGNAL(valueChanged(int)), this, SLOT(scrollY(int)));
	connect(hScrollBar_, SIGNAL(valueChanged(int)), this, SLOT(scrollX(int)));
	connect(canvas_, SIGNAL(mousePressEvent(QMouseEvent *)), this, SIGNAL(mousePressEvent(QMouseEvent *)));
	connect(this, SIGNAL(mousePressEvent(QMouseEvent *)), this, SLOT(processMouseEvent(QMouseEvent *)));
	connect(canvas_, SIGNAL(wheelEvent(QWheelEvent *)), this, SIGNAL(wheelEvent(QWheelEvent *)));
	connect(this, SIGNAL(wheelEvent(QWheelEvent *)), this, SLOT(processWheelEvent(QWheelEvent *)));
	
	layout_ = new QGridLayout(this);
	layout_->setMargin(0);
	layout_->addWidget(canvas_, 0, 0);
	layout_->addWidget(vScrollBar_, 0, 1);
	layout_->addWidget(hScrollBar_, 1, 0);
	this->setLayout(layout_);
}

void CAScrollWidget::paintEvent(QPaintEvent *e) {
	checkScrollBars();	
}

/**
 * Called on mouse events like left/right mouse button.
 * Canvas mouse events are redirected to this method.
 * 
 * @param QMouseEvent *e Mouse event
 */
void CAScrollWidget::processMouseEvent(QMouseEvent *e) {
	int worldX = (int)(e->x()/canvas_->zoom()) + canvas_->worldX();
	int worldY = (int)(e->y()/canvas_->zoom()) + canvas_->worldY();
	
	if (e->modifiers()==Qt::ControlModifier) {
		musElements_.removeElement(worldX, worldY);
	} else {
		musElements_.addElement(new CANote(4, worldX, worldY));
	}
	
	repaint();
}

/**
 * Called on wheel (mouse wheel) events.
 * Canvas wheel events are redirected to this method.
 * 
 * @param QWheelEvent *e Wheel event
 */
void CAScrollWidget::processWheelEvent(QWheelEvent *e) {
	int v;
	switch (e->modifiers()) {
		case Qt::NoModifier:			//scroll horizontally
			if (!hScrollBar_->isVisible()) break;
			v = hScrollBar_->value();
			v = (int)(v - e->delta()*0.5/canvas_->zoom());
			if (v < 0) v = 0;
			if (v > hScrollBar_->maximum()) v = hScrollBar_->maximum();
			scrollX(v);
			break;
		case Qt::AltModifier:			//scroll horizontally, fast
			if (!hScrollBar_->isVisible()) break;
			v = hScrollBar_->value();
			v = (int)(v - e->delta()/canvas_->zoom());
			if (v < 0) v = 0;
			if (v > hScrollBar_->maximum()) v = hScrollBar_->maximum();
			scrollX(v);
			break;
		case Qt::ShiftModifier:			//scroll vertically
			if (!vScrollBar_->isVisible()) break;
			v = vScrollBar_->value();
			v = (int)(v - e->delta()*0.5/canvas_->zoom());
			if (v < 0) v = 0;
			if (v > vScrollBar_->maximum()) v = vScrollBar_->maximum();
			scrollY(v);
			break;
		case 0x0A000000://SHIFT+ALT		//scroll vertically, fast
			if (!hScrollBar_->isVisible()) break;
			v = vScrollBar_->value();
			v = (int)(v - e->delta()/canvas_->zoom());
			if (v < 0) v = 0;			
			if (v > vScrollBar_->maximum()) v = vScrollBar_->maximum();
			scrollY(v);
			break;
		case Qt::ControlModifier:		//zoom
			if (e->delta()>0)
				canvas_->setZoom(canvas_->zoom()*1.1, e->x(), e->y());
			else
				canvas_->setZoom(canvas_->zoom()/1.1, e->x(), e->y());
			
			break;
	}	
}

/**
 * Update scrollbars' maximum values.
 * Show/hide scrollbars, if needed.
 */
void CAScrollWidget::checkScrollBars() {
	hScrollBar_->setValue(canvas_->worldX());
	vScrollBar_->setValue(canvas_->worldY());
	if (musElements_.getMaxX() - canvas_->worldWidth() > 0) { //if scrollbar is needed
		hScrollBar_->setMaximum(musElements_.getMaxX() - canvas_->worldWidth());
		hScrollBar_->show();
	} else //if the whole scene can be drawn on the canvas
		if (hScrollBar_->value()==0)
			hScrollBar_->hide();
		
	if (musElements_.getMaxY() - canvas_->worldHeight() > 0) { //if scrollbar is needed
		vScrollBar_->setMaximum(musElements_.getMaxY() - canvas_->worldHeight());
		vScrollBar_->show();
	} else //if the whole scene can be drawn on the canvas
		if (vScrollBar_->value()==0)
			vScrollBar_->hide();
}

/**
 * Scroll to given X position. Repaint the view.
 * 
 * @param x X position in world units
 */
void CAScrollWidget::scrollX(int x) {
	canvas_->setWorldX(x);
	canvas_->repaint();
	hScrollBar_->setValue(x);
}

/**
 * Scroll to given Y position. Repaint the view.
 * 
 * @param y Y position in world units
 */
void CAScrollWidget::scrollY(int y) {
	canvas_->setWorldY(y);
	canvas_->repaint();
	vScrollBar_->setValue(y);
}

/**
 * Scroll to given X, Y position. Repaint the view.
 * 
 * @param x X position in world units
 * @param y Y position in world units
 */
void CAScrollWidget::scroll(int x, int y) {
	canvas_->setWorldCoords(x, y);
	hScrollBar_->setValue(x);
	vScrollBar_->setValue(y);
}

/**
 * Scroll to the given element so the element is in the center of the viewport.
 *
 * @param CADrawable *elt - pointer to the given element
 */
void CAScrollWidget::scrollToElement(CADrawable *elt) {
	canvas_->setCenterCoords(elt->xPos(), elt->yPos());
}

/**
 * Set zoom level.
 * 
 * @param float z Zoom level (100% = 1.0, 50% = 0.5 etc.)
 */
void CAScrollWidget::setZoom(float z, int x, int y) {
	canvas_->setZoom(z, x, y);
}
