/** @file viewport.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QScrollBar>

#include "viewport.h"

CAViewPort::CAViewPort(CAKDTree *t, QWidget *p) : QWidget(p) {
	_parent = p;
	_worldX = _worldY = 0;
	_zoom = 1.0;
	_musElements = t;
	_holdRepaint = false;
	_hScrollBarDeadLock = false;
	_vScrollBarDeadLock = false;
	_checkScrollBarsDeadLock = false;
	
	//setup the mouse events and forward them to CAEventName events
	connect(this, SIGNAL(mousePressEvent(QMouseEvent *)), this, SLOT(processMousePressEvent(QMouseEvent *)));
	connect(this, SIGNAL(wheelEvent(QWheelEvent *)), this, SLOT(processWheelEvent(QWheelEvent *)));

	//setup the virtual canvas
	_canvas = new QWidget(this);

	//setup the scrollbars
	_vScrollBar = new QScrollBar(Qt::Vertical, this);
	_hScrollBar = new QScrollBar(Qt::Horizontal, this);
	_vScrollBar->setMinimum(0);
	_hScrollBar->setMinimum(0);
	_vScrollBar->setTracking(true); //trigger valueChanged() when dragging the slider, not only releasing it
	_hScrollBar->setTracking(true);
	_vScrollBar->hide();
	_hScrollBar->hide();
	_scrollBarsVisible = ScrollBarShowIfNeeded;
	_allowManualScroll = true;
	
	connect(_hScrollBar, SIGNAL(valueChanged(int)), this, SLOT(processHScrollBarEvent(int)));
	connect(_vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(processVScrollBarEvent(int)));
	
	//setup layout
	_layout = new QGridLayout(this);
	_layout->setMargin(0);
	_layout->addWidget(_canvas, 0, 0);
	_layout->addWidget(_vScrollBar, 0, 1);
	_layout->addWidget(_hScrollBar, 1, 0);
	
	_oldWorldW = 0; _oldWorldH = 0;
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAViewPort::setWorldX(int x, bool force) {
	if (!force && _musElements) {
		int maxX = _musElements->getMaxX();
		if (x > maxX - _worldW)
			x = maxX - _worldW;
		if (x < 0)
			x = 0;
	}
	
	_oldWorldX = _worldX;
	_worldX = x;
	_hScrollBarDeadLock = true;
	_hScrollBar->setValue(x);
	_hScrollBarDeadLock = false;

	checkScrollBars();
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAViewPort::setWorldY(int y, bool force) {
	if (!force && _musElements) {
		int maxY = _musElements->getMaxY();
		if (y > maxY - _worldH)
			y = maxY - _worldH;
		if (y < 0)
			y = 0;
	}
	
	_oldWorldY = _worldY;
	_worldY = y;
	_vScrollBarDeadLock = true;
	_vScrollBar->setValue(y);
	_vScrollBarDeadLock = false;
	
	checkScrollBars();
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAViewPort::setWorldWidth(int w, bool force) {
	if (!force) {
		if (w < 1) return;
	}
	
	_oldWorldW = _worldW;
	_worldW = w;
	
	int scrollMax;
	if ((_musElements) && ((scrollMax = _musElements->getMaxX() - _worldW) > 0)) {
		_hScrollBarDeadLock = true;
		_hScrollBar->setMaximum(scrollMax);
		_hScrollBar->setPageStep(_worldH);
		_hScrollBarDeadLock = false;
	}
	
	_zoom = ((float)drawableWidth() / _worldW);

	checkScrollBars();
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAViewPort::setWorldHeight(int h, bool force) {
	if (!force) {
		if (h < 1) return;
	}
	
	_oldWorldH = _worldH;
	_worldH = h;

	int scrollMax;
	if ((_musElements) && ((scrollMax = _musElements->getMaxY() - _worldH) > 0)) {
		_vScrollBarDeadLock = true;
		_vScrollBar->setMaximum(scrollMax);
		_vScrollBar->setPageStep(_worldH);
		_vScrollBarDeadLock = false;
	}

	_zoom = ((float)drawableHeight() / _worldH);

	checkScrollBars();
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAViewPort::setWorldCoords(int x, int y, int w, int h, bool force) {
	setWorldX(x, force);
	setWorldY(y, force);
	setWorldWidth(w, force);
	setWorldHeight(h, force);
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAViewPort::setWorldCoords(int x, int y, bool force) {
	setWorldX(x, force);
	setWorldY(y, force);
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAViewPort::setCenterCoords(int x, int y, bool force) {
	setWorldX(x - (int)(0.5*_worldW), force);
	setWorldY(y - (int)(0.5*_worldH), force);
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAViewPort::setZoom(float z, int x, int y, bool force) {
	bool zoomOut = false;
	if (_zoom - z > 0.0)
		zoomOut = true;

	//set the world width - updates the zoom level zoom_ as well
	setWorldWidth((int)(drawableWidth() / z));
	setWorldHeight((int)(drawableHeight() / z));
	
	if (!zoomOut) { //zoom in
		//the new view's center coordinates will become the middle point of the current viewport center coords and the mouse pointer coords
		setCenterCoords( ( _worldX + (_worldW/2) + x ) / 2,
		                 ( _worldY + (_worldH/2) + y ) / 2,
		                 force );
	} else { //zoom out
		//the new view's center coordinates will become the middle point of the current viewport center coords and the mirrored over center pointer coords
		//worldX_ + (worldW_/2) + (worldX_ + (worldW_/2) - x)/2
		setCenterCoords( (int)(1.5*_worldX + 0.75*_worldW - 0.5*x),
		                 (int)(1.5*_worldY + 0.75*_worldH - 0.5*y),
		                 force );
	}
	
	checkScrollBars();
}

/**
 * WARNING! This method is called when the virtual _canvas gets resized as well!
 */
void CAViewPort::paintEvent(QPaintEvent *e) {
	if (!_musElements || _holdRepaint)
		return;
	
	QPainter p(this);
	p.drawLine(0, 0, drawableWidth(), drawableHeight());
	QList<CADrawable *>* l = _musElements->findInRange(_worldX, _worldY, _worldW, _worldH);
	if (!l) return;

	for (int i=0; i<l->size(); i++) {
		l->at(i)->draw(&p,
		               (int)((l->at(i)->xPos() - _worldX) * _zoom),
		               (int)((l->at(i)->yPos() - _worldY) * _zoom),
		               _zoom);
	}
	
	//flush the oldWorld coordinates as they're needed for the first repaint only
	_oldWorldX = _worldX; _oldWorldY = _worldY;
	_oldWorldW = _worldW; _oldWorldH = _worldH;	
}

void CAViewPort::resizeEvent(QResizeEvent *e) {
	setWorldWidth( (int)(drawableWidth() / _zoom) );
	setWorldHeight( (int)(drawableHeight() / _zoom) );
	
	checkScrollBars();
}

void CAViewPort::checkScrollBars() {
	if ((_scrollBarsVisible != ScrollBarShowIfNeeded) || (!_musElements) || (_checkScrollBarsDeadLock))
		return;
	
	bool change = false;
	_holdRepaint = true;	//disable repaint until the scrollbar values are set
	_checkScrollBarsDeadLock = true;	//disable any further method calls until the method is over
	if ((_musElements->getMaxX() - worldWidth() > 0) || (_hScrollBar->value()!=0)) { //if scrollbar is needed
		if (!_hScrollBar->isVisible()) {
			_hScrollBar->show();
			change = true;
		}
	} else //if the whole scene can be drawn on the canvas and the scrollbars are at position 0
		if (_hScrollBar->isVisible()) {
			_hScrollBar->hide();
			change = true;
		}
		
	if ((_musElements->getMaxY() - worldHeight() > 0) || (_vScrollBar->value()!=0)) { //if scrollbar is needed
		if (!_vScrollBar->isVisible()) {
			_vScrollBar->show();
			change = true;
		}
	} else //if the whole scene can be drawn on the canvas and the scrollbars are at position 0
		if (_vScrollBar->isVisible()) {
			_vScrollBar->hide();
			change = true;
		}
	
	if (change) {
		setWorldHeight((int)(drawableHeight() / _zoom));
		setWorldWidth((int)(drawableWidth() / _zoom));
	}

	_holdRepaint = false;
	_checkScrollBarsDeadLock = false;
}

void CAViewPort::processMousePressEvent(QMouseEvent *e) {
	emit CAMousePressEvent(e, QPoint((int)(e->x() / _zoom) + _worldX, (int)(e->y() / _zoom) + _worldY), this);
}

void CAViewPort::processWheelEvent(QWheelEvent *e) {
	emit CAWheelEvent(e, QPoint((int)(e->x() / _zoom) + _worldX, (int)(e->y() / _zoom) + _worldY), this);	
}

void CAViewPort::setScrollBarsVisibility(char status) {
	_scrollBarsVisible = status;
	
	if ((status == ScrollBarAlwaysVisible) && (!_hScrollBar->isVisible())) {
		_hScrollBar->show();
		_vScrollBar->show();
		return;
	}
	
	if ((status == ScrollBarAlwaysHidden) && (_hScrollBar->isVisible())) {
		_hScrollBar->hide();
		_vScrollBar->hide();
		return;
	}
	
	checkScrollBars();
}

void CAViewPort::processHScrollBarEvent(int val) {
	if ((_allowManualScroll) && (!_hScrollBarDeadLock)) {
		setWorldX(val);
		repaint();
	}
}

void CAViewPort::processVScrollBarEvent(int val) {
	if ((_allowManualScroll) && (!_vScrollBarDeadLock)) {
		setWorldY(val);
		repaint();
	}
}
