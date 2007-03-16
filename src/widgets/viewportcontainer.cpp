/** @file widgets/scrollwidget.cpp
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
#include <QGridLayout>

#include "widgets/viewportcontainer.h"
#include "widgets/viewport.h"
#include "widgets/multisplitter.h"
#include "core/sheet.h"
#include "core/note.h"
#include "core/staff.h"

CAViewPortContainer::CAViewPortContainer(CAViewPort *v, QWidget *parent) : QFrame(parent) {
	_layout = new QGridLayout(this); _layout->setSpacing(0); _layout->setMargin(0);
	_splitter = new CAMultiSplitter(v, this);
	
	_viewPorts.append(_lastUsedViewPort = v);
	
	_layout->addWidget(_splitter->main());
}

CAViewPortContainer::~CAViewPortContainer()
{
	delete _splitter;
	delete _layout;
}

void CAViewPortContainer::paintEvent(QPaintEvent *e) {
}

void CAViewPortContainer::resizeEvent(QResizeEvent *) {
}

CAViewPort* CAViewPortContainer::splitVertically(CAViewPort *v) {
	if(v) return 0;
	if(_lastUsedViewPort->viewPortType() == CAViewPort::ScoreViewPort)
		v = _lastUsedViewPort;
	else
		v = (CAViewPort*)_splitter->lastUsedWidget();
	_viewPorts.append(_lastUsedViewPort = v->clone());
	_splitter->addWidget(_lastUsedViewPort, Qt::Vertical);
	
	return _lastUsedViewPort;
}

CAViewPort* CAViewPortContainer::splitHorizontally(CAViewPort *v) {
	if(v) return 0;
	if(_lastUsedViewPort->viewPortType() == CAViewPort::ScoreViewPort)
		v = _lastUsedViewPort;
	else
		v = (CAViewPort*)_splitter->lastUsedWidget(); 
	_viewPorts.append(_lastUsedViewPort = v->clone());
	_splitter->addWidget(_lastUsedViewPort, Qt::Horizontal);
	
	return _lastUsedViewPort;
}

CAViewPort* CAViewPortContainer::unsplit(CAViewPort *v) {
	CAViewPort* rv = 0;
	if (_viewPorts.size() <= 1)
		return 0;
	
	if (!v)
		v = _lastUsedViewPort;
	
	if(v->viewPortType() == CAViewPort::ScoreViewPort)
	{
		if(_splitter->widgetCount() > 1)
			_splitter->removeWidget(v);
		else
			return 0; //do nothing if this is the only score viewport.
	}
	
	if (_viewPorts.removeAll(v))
		rv = v;
	
	_lastUsedViewPort = _viewPorts.back();
	_splitter->setLastUsedWidget(_lastUsedViewPort);
	return rv;
}

QList<CAViewPort*> CAViewPortContainer::unsplitAll() {
	QList<CAViewPort*> list;
	while(_splitter->widgetCount() > 1)
		list << unsplit();
	if(_viewPorts.size() == 2) //if _splitter->widgetCount() == 1, then _viewPorts.size() can be either 1 or 2 (source viewport)
	{
		//find which of the viewports is the source viewport
		CAViewPort *v = unsplit(_viewPorts[0]);
		if(!v)
			v = unsplit(_viewPorts[1]);
		list << v;
	}
	return list;
}

CAViewPort* CAViewPortContainer::newViewPort(CAViewPort *v) {
	CAViewPort* viewPort = v?v->clone(0):_lastUsedViewPort->clone(0);
	
	//set the _worldW, _worldH and update scrollbars etc. beside the size
	viewPort->resize( ((v && (v->width() > DEFAULT_VIEWPORT_WIDTH))?v->width():DEFAULT_VIEWPORT_WIDTH),
	                  ((v && (v->height() > DEFAULT_VIEWPORT_HEIGHT))?v->height():DEFAULT_VIEWPORT_HEIGHT)
	                );
	viewPort->show();
	return viewPort;
}

void CAViewPortContainer::addViewPort(CAViewPort *v) {
	_viewPorts.append(_lastUsedViewPort = v);
	
	//usually used for source viewports.
	if(_splitter->main()->orientation() == Qt::Vertical)
		_splitter->main()->addWidget(v);
	else 
	{
		if(_layout->itemAt(0)->widget() == _splitter->main())
		{
			QSplitter *container = new QSplitter(this);
			container->setOrientation(Qt::Vertical);
			container->addWidget(_splitter->main());
			container->addWidget(v);
			
			_layout->takeAt(0);
			_layout->addWidget(container);
		} else
			//There was once a source viewport, so there's already a container.
			static_cast<QSplitter*>(_layout->itemAt(0)->widget())->addWidget(v);
	}
}

void CAViewPortContainer::setLastUsedViewPort(CAViewPort* v)
{
	if(_viewPorts.contains(v))
	{
		_lastUsedViewPort = v; 
		if(v->viewPortType() == CAViewPort::ScoreViewPort) 
			_splitter->setLastUsedWidget((QWidget*)v); 
	} 
}
