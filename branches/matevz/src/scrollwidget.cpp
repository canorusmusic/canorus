#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>

#include "scrollwidget.h"
#include "note.h"

using namespace std;

CAScrollWidget::CAScrollWidget(QWidget *parent) : QFrame(parent) {
	viewPort_ = new CAViewPort(this, &musElements_);
	
	connect(viewPort_, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
	connect(viewPort_, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));
	
	layout_ = new QGridLayout(this);
	layout_->setMargin(0);
	layout_->addWidget(viewPort_, 0, 0);
//	layout_->addWidget(new CAViewPort(this, &musElements_), 1, 0);
	this->setLayout(layout_);
}

void CAScrollWidget::paintEvent(QPaintEvent *e) {
}

/**
 * Called on mouse events like left/right mouse button in children viewports.
 * 
 * @param QMouseEvent *e Mouse event
 * @param QPoint coords Mouse coordinates where the event happened in world units
 * @param CAViewPort *v Pointer to the viewport where the event happened
 */
void CAScrollWidget::viewPortMousePressEvent(QMouseEvent *e, QPoint coords, CAViewPort *v) {
	if (e->modifiers()==Qt::ControlModifier) {
		musElements_.removeElement(coords.x(), coords.y());
	} else {
		musElements_.addElement(new CANote(4, coords.x(), coords.y()));
	}
	
	v->checkScrollBars();
	v->repaint();	
}

/**
 * Called on wheel (mouse wheel) events in the children viewports.
 * 
 * @param QWheelEvent *e Wheel event
 * @param QPoint coords Mouse coordinates where the event happened in world units
 * @param CAViewPort *v Pointer to the viewport where the event happened
 */
void CAScrollWidget::viewPortWheelEvent(QWheelEvent *e, QPoint coords, CAViewPort *v) {
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
				v->setZoom( v->zoom()*1.1,
				            (int)(e->x()/v->zoom()) + v->worldX(),
				            (int)(e->y()/v->zoom()) + v->worldY() );
			else
				v->setZoom( v->zoom()/1.1,
				            (int)(e->x()/v->zoom()) + v->worldX(),
				            (int)(e->y()/v->zoom()) + v->worldY() );
			
			break;
	}

	v->repaint();
}

void CAScrollWidget::resizeEvent(QResizeEvent *) {
}
