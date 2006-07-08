#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPushButton>
#include <iostream>
#include "scrollwidget.h"
#include "note.h"

using namespace std;

CAScrollWidget::CAScrollWidget(QWidget *parent) : QFrame(parent) {
	CAViewPort *v;
	viewPorts_.push_back(v = lastUsedViewPort_ = new CAViewPort(this, &musElements_));
	
	connect(v, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
	connect(v, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));
	
	layout_ = new QGridLayout(this);
	layout_->setMargin(0);
	layout_->addWidget(v, 0, 0);
}

void CAScrollWidget::paintEvent(QPaintEvent *e) {
}

/**
 * Called on mouse events like left/right mouse button in children viewports.
 * 
 * @param QMouseEvent *e Mouse event
 * @param QPoint coords Mouse coordinates where the event happened in absolute world units
 * @param CAViewPort *v Pointer to the viewport where the event happened
 */
void CAScrollWidget::viewPortMousePressEvent(QMouseEvent *e, QPoint coords, CAViewPort *v) {
	lastUsedViewPort_ = v;

	if (e->modifiers()==Qt::ControlModifier) {
		musElements_.removeElement(coords.x(), coords.y());
	} else {
		musElements_.addElement(new CANote(4, coords.x(), coords.y()));
	}
	
	for (int i=0; i<viewPorts_.size(); i++) {
		viewPorts_[i]->checkScrollBars();
		viewPorts_[i]->repaint();
	}
}

/**
 * Called on wheel (mouse wheel) events in the children viewports.
 * 
 * @param QWheelEvent *e Wheel event
 * @param QPoint coords Mouse coordinates where the event happened in absolute world units
 * @param CAViewPort *v Pointer to the viewport where the event happened
 */
void CAScrollWidget::viewPortWheelEvent(QWheelEvent *e, QPoint coords, CAViewPort *v) {
	lastUsedViewPort_ = v;

	int val;
	switch (e->modifiers()) {
		case Qt::NoModifier:			//scroll horizontally
			v->setWorldX(v->worldX() - (int)(e->delta()*0.5/v->zoom()));
			break;
		case Qt::AltModifier:			//scroll horizontally, fast
			v->setWorldX(v->worldX() - (int)(e->delta()/v->zoom()));
			break;
		case Qt::ShiftModifier:			//scroll vertically
			v->setWorldY(v->worldY() - (int)(e->delta()*0.5/v->zoom()));
			break;
		case 0x0A000000://SHIFT+ALT		//scroll vertically, fast
			v->setWorldY(v->worldY() - (int)(e->delta()/v->zoom()));
			break;
		case Qt::ControlModifier:		//zoom
			if (e->delta()>0)
				v->setZoom( v->zoom()*1.1, coords.x(), coords.y() );
			else
				v->setZoom( v->zoom()/1.1, coords.x(), coords.y() );
			
			break;
	}

	v->repaint();
}

void CAScrollWidget::resizeEvent(QResizeEvent *) {
}

void CAScrollWidget::splitVertically(CAViewPort *v) {
	if (!v) {
		viewPorts_.append(lastUsedViewPort_ = new CAViewPort(this, &musElements_));
		connect(lastUsedViewPort_, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
		connect(lastUsedViewPort_, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));
		
		layout_->addWidget(lastUsedViewPort_, 0, layout_->columnCount(), layout_->rowCount(), 1);
		layout_->update();
	}
}

void CAScrollWidget::splitHorizontally(CAViewPort *v) {
	if (!v) {
		viewPorts_.append(lastUsedViewPort_ =  new CAViewPort(this, &musElements_));
		connect(lastUsedViewPort_, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
		connect(lastUsedViewPort_, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));

		layout_->addWidget(lastUsedViewPort_, layout_->rowCount(), 0, 1, layout_->columnCount());
		layout_->update();
	}
}

void CAScrollWidget::unsplit(CAViewPort *v) {
	if (viewPorts_.size() <= 1)
		return;
	
	viewPorts_.removeAll(v?v:lastUsedViewPort_);
	layout_->removeWidget(v?v:lastUsedViewPort_);
	delete (v?v:lastUsedViewPort_);
	
	lastUsedViewPort_ = viewPorts_.back();
}

void CAScrollWidget::newViewPort(CAViewPort *v) {
	viewPorts_.append(lastUsedViewPort_ =  new CAViewPort(0, &musElements_));
	lastUsedViewPort_->resize(v?v->width():400, v?v->height():400);
	connect(lastUsedViewPort_, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
	connect(lastUsedViewPort_, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));

	lastUsedViewPort_->show();
}
