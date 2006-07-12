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
#include "staff.h"

CAScrollWidget::CAScrollWidget(CAViewPort *v, QWidget *parent) : QFrame(parent) {
	_layout = new QGridLayout(this);
	_layout->setMargin(0);

	_viewPorts.append(_lastUsedViewPort = v);
	
	_layout->addWidget(_lastUsedViewPort, 0, 0);
}

void CAScrollWidget::paintEvent(QPaintEvent *e) {
}

void CAScrollWidget::resizeEvent(QResizeEvent *) {
}

CAViewPort* CAScrollWidget::splitVertically(CAViewPort *v) {
	if (!v) {
		_viewPorts.append(_lastUsedViewPort = _lastUsedViewPort->clone());
		connect(_lastUsedViewPort, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
		connect(_lastUsedViewPort, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));
		
		_layout->addWidget(_lastUsedViewPort, 0, _layout->columnCount(), _layout->rowCount(), 1);
		_layout->update();
		
		return _lastUsedViewPort;
	}
}

CAViewPort* CAScrollWidget::splitHorizontally(CAViewPort *v) {
	if (!v) {
		_viewPorts.append(_lastUsedViewPort = _lastUsedViewPort->clone());
		connect(_lastUsedViewPort, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
		connect(_lastUsedViewPort, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));

		_layout->addWidget(_lastUsedViewPort, _layout->rowCount(), 0, 1, _layout->columnCount());
		_layout->update();
		
		return _lastUsedViewPort;
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

CAViewPort* CAScrollWidget::newViewPort(CAViewPort *v) {
	_viewPorts.append(_lastUsedViewPort =  v?v->clone(0):_lastUsedViewPort->clone(0));
	connect(_lastUsedViewPort, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
	connect(_lastUsedViewPort, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));

	_lastUsedViewPort->show();
	//set the _worldW, _worldH and update scrollbars etc. beside the size
	_lastUsedViewPort->resize(v?v->width():DEFAULT_VIEWPORT_WIDTH, v?v->height():DEFAULT_VIEWPORT_HEIGHT);
	
	return _lastUsedViewPort;
}
