/** @file scrollwidget.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPushButton>

#include "viewport.h"
#include "sheet.h"
#include "scrollwidget.h"
#include "note.h"

CAScrollWidget::CAScrollWidget(CASheet *s, QWidget *parent) : QFrame(parent) {
	_layout = new QGridLayout(this);
	_layout->setMargin(0);
	_sheet = s;

	_viewPorts.append(_lastUsedViewPort = new CAViewPort(&_musElements, this));
	connect(_lastUsedViewPort, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
	connect(_lastUsedViewPort, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));
	
	_layout->addWidget(_lastUsedViewPort, 0, 0);
}

void CAScrollWidget::paintEvent(QPaintEvent *e) {
}

void CAScrollWidget::viewPortMousePressEvent(QMouseEvent *e, QPoint coords, CAViewPort *v) {
	_lastUsedViewPort = v;

	if (e->modifiers()==Qt::ControlModifier) {
		_musElements.removeElement(coords.x(), coords.y());
	} else {
		_musElements.addElement(new CANote(4, coords.x(), coords.y()));
	}
	
	for (int i=0; i<_viewPorts.size(); i++) {
		_viewPorts[i]->checkScrollBars();
		_viewPorts[i]->repaint();
	}
}

void CAScrollWidget::viewPortWheelEvent(QWheelEvent *e, QPoint coords, CAViewPort *v) {
	_lastUsedViewPort = v;

	int val;
	switch (e->modifiers()) {
		case Qt::NoModifier:			//scroll horizontally
			v->setWorldX( v->worldX() - (int)((0.5*e->delta()) / v->zoom()) );
			break;
		case Qt::AltModifier:			//scroll horizontally, fast
			v->setWorldX( v->worldX() - (int)(e->delta() / v->zoom()) );
			break;
		case Qt::ShiftModifier:			//scroll vertically
			v->setWorldY( v->worldY() - (int)((0.5*e->delta()) / v->zoom()) );
			break;
		case 0x0A000000://SHIFT+ALT		//scroll vertically, fast
			v->setWorldY( v->worldY() - (int)(e->delta() / v->zoom()) );
			break;
		case Qt::ControlModifier:		//zoom
			if (e->delta() > 0)
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
		_viewPorts.append(_lastUsedViewPort = new CAViewPort(&_musElements, this));
		connect(_lastUsedViewPort, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
		connect(_lastUsedViewPort, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));
		
		_layout->addWidget(_lastUsedViewPort, 0, _layout->columnCount(), _layout->rowCount(), 1);
		_layout->update();
	}
}

void CAScrollWidget::splitHorizontally(CAViewPort *v) {
	if (!v) {
		_viewPorts.append(_lastUsedViewPort =  new CAViewPort(&_musElements, this));
		connect(_lastUsedViewPort, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
		connect(_lastUsedViewPort, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));

		_layout->addWidget(_lastUsedViewPort, _layout->rowCount(), 0, 1, _layout->columnCount());
		_layout->update();
	}
}

void CAScrollWidget::unsplit(CAViewPort *v) {
	if (_viewPorts.size() <= 1)
		return;
	
	_viewPorts.removeAll(v?v:_lastUsedViewPort);
	(v?v:_lastUsedViewPort)->disconnect();	//disconnect all the signals
	_layout->removeWidget(v?v:_lastUsedViewPort);
	delete (v?v:_lastUsedViewPort);
	
	_lastUsedViewPort = _viewPorts.back();
}

void CAScrollWidget::newViewPort(CAViewPort *v) {
	_viewPorts.append(_lastUsedViewPort =  new CAViewPort(&_musElements, 0));
	connect(_lastUsedViewPort, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
	connect(_lastUsedViewPort, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));

	_lastUsedViewPort->show();
	//set the _worldW, _worldH and update scrollbars etc. beside the size
	_lastUsedViewPort->resize(v?v->width():DEFAULT_VIEWPORT_WIDTH, v?v->height():DEFAULT_VIEWPORT_HEIGHT);
}
