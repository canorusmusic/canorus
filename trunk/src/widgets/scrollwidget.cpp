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
#include <QSplitter>
#include <QGridLayout>

#include "widgets/viewport.h"
#include "core/sheet.h"
#include "widgets/scrollwidget.h"
#include "core/note.h"
#include "core/staff.h"

CAScrollWidget::CAScrollWidget(CAViewPort *v, QWidget *parent) : QFrame(parent) {
	_layout = new QGridLayout(this); _layout->setSpacing(0); _layout->setMargin(0);
	_splitter = new QSplitter(this);

	_viewPorts.append(_lastUsedViewPort = v);
	
	_splitter->addWidget(_lastUsedViewPort);
	_layout->addWidget(_splitter);
}

void CAScrollWidget::paintEvent(QPaintEvent *e) {
}

void CAScrollWidget::resizeEvent(QResizeEvent *) {
}

CAViewPort* CAScrollWidget::splitVertically(CAViewPort *v) {
	if (!v) {
		_viewPorts.append(_lastUsedViewPort = _lastUsedViewPort->clone());
		
		_splitter->setOrientation(Qt::Horizontal);
		_splitter->addWidget(_lastUsedViewPort);
		_splitter->update();
		
		return _lastUsedViewPort;
	}
}

CAViewPort* CAScrollWidget::splitHorizontally(CAViewPort *v) {
	if (!v) {
		_viewPorts.append(_lastUsedViewPort = _lastUsedViewPort->clone());

		_splitter->setOrientation(Qt::Vertical);
		_splitter->addWidget(_lastUsedViewPort);
		_splitter->update();
		
		return _lastUsedViewPort;
	}
}

CAViewPort* CAScrollWidget::unsplit(CAViewPort *v) {
	if (_viewPorts.size() <= 1)
		return 0;
	
	if (!v)
		v = _lastUsedViewPort;
		
	v->disconnect();	//disconnect all the signals
	delete (v);
	
	
	if (_viewPorts.removeAll(v)) {
		_lastUsedViewPort = _viewPorts.back();
		return v;
	} else {
		_lastUsedViewPort = _viewPorts.back();
		return 0;
	}
}

CAViewPort* CAScrollWidget::newViewPort(CAViewPort *v) {
	CAViewPort* viewPort;
	_viewPorts.append(viewPort = v?v->clone(0):_lastUsedViewPort->clone(0));

	viewPort->show();
	//set the _worldW, _worldH and update scrollbars etc. beside the size
	viewPort->resize( ((v && (v->width() > DEFAULT_VIEWPORT_WIDTH))?v->width():DEFAULT_VIEWPORT_WIDTH),
	                  ((v && (v->height() > DEFAULT_VIEWPORT_HEIGHT))?v->height():DEFAULT_VIEWPORT_HEIGHT)
	                );
	
	return viewPort;
}

void CAScrollWidget::addViewPort(CAViewPort *v) {
	_viewPorts.append(_lastUsedViewPort = v);
	
	_splitter->setOrientation(Qt::Vertical);
	_splitter->addWidget(_lastUsedViewPort);
	_splitter->update();
}
