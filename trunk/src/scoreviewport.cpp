/** @file scoreviewport.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QGridLayout>
#include <QScrollBar>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>

#include <iostream>

#include "scoreviewport.h"
#include "drawable.h"
#include "muselement.h"
#include "context.h"
#include "engraver.h"

CAScoreViewPort::CAScoreViewPort(CASheet *sheet, QWidget *parent) : CAViewPort(parent) {
	_viewPortType = CAViewPort::ScoreViewPort;
	
	_sheet = sheet;
	_worldX = _worldY = 0;
	_zoom = 1.0;
	_holdRepaint = false;
	_hScrollBarDeadLock = false;
	_vScrollBarDeadLock = false;
	_checkScrollBarsDeadLock = false;
	
	_currentContext = 0;
	
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

CAScoreViewPort::~CAScoreViewPort() {
	_drawableMList.clear(true);	//clears all the elements and delete its contents too as autoDelete is true
	_drawableCList.clear(true);	//clears all the elements and delete its contents too as autoDelete is true
	
	_hScrollBar->disconnect();
	_vScrollBar->disconnect();
	this->disconnect();
}

CAScoreViewPort *CAScoreViewPort::clone() {
	CAScoreViewPort *v = new CAScoreViewPort(_sheet, _parent);
	
	v->importMElements(&_drawableMList);
	v->importCElements(&_drawableCList);
	
	return v;
}

CAScoreViewPort *CAScoreViewPort::clone(QWidget *parent) {
	CAScoreViewPort *v = new CAScoreViewPort(_sheet, parent);
	
	v->importMElements(&_drawableMList);
	v->importCElements(&_drawableCList);
	
	return v;
}

void CAScoreViewPort::addMElement(CADrawable *elt) {
	_drawableMList.addElement(elt);
}

void CAScoreViewPort::addCElement(CADrawable *elt) {
	_drawableCList.addElement(elt);
}

void CAScoreViewPort::importMElements(CAKDTree *elts) {
	_drawableMList.import(elts);
}

void CAScoreViewPort::importCElements(CAKDTree *elts) {
	_drawableCList.import(elts);
}

void CAScoreViewPort::update() {
	_drawableMList.clear(true);
	_drawableCList.clear(true);
	
	CAEngraver::reposit(this);
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAScoreViewPort::setWorldX(int x, bool force) {
	if (!force) {
		int maxX = (_drawableMList.getMaxX() > _drawableCList.getMaxX())?_drawableMList.getMaxX() : _drawableCList.getMaxX();
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
void CAScoreViewPort::setWorldY(int y, bool force) {
	if (!force) {
		int maxY = (_drawableMList.getMaxY() > _drawableCList.getMaxY())?_drawableMList.getMaxY() : _drawableCList.getMaxY();
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
void CAScoreViewPort::setWorldWidth(int w, bool force) {
	if (!force) {
		if (w < 1) return;
	}
	
	_oldWorldW = _worldW;
	_worldW = w;
	
	int scrollMax;
	if ((scrollMax = ((_drawableMList.getMaxX() > _drawableCList.getMaxX())?_drawableMList.getMaxX():_drawableCList.getMaxX()) - _worldW) >= 0) {
		if (scrollMax < _worldX)	//if you resize the widget at a large zoom level and if the getMax border has been reached
			setWorldX(scrollMax);	//scroll the view away from the border
			
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
void CAScoreViewPort::setWorldHeight(int h, bool force) {
	if (!force) {
		if (h < 1) return;
	}
	
	_oldWorldH = _worldH;
	_worldH = h;

	int scrollMax;
	if ((scrollMax = ((_drawableMList.getMaxY() > _drawableCList.getMaxY())?_drawableMList.getMaxY():_drawableCList.getMaxY()) - _worldH) >= 0) {
		if (scrollMax < _worldY)	//if you resize the widget at a large zoom level and if the getMax border has been reached
			setWorldY(scrollMax);	//scroll the view away from the border
		
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
void CAScoreViewPort::setWorldCoords(int x, int y, int w, int h, bool force) {
	setWorldX(x, force);
	setWorldY(y, force);
	setWorldWidth(w, force);
	setWorldHeight(h, force);
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAScoreViewPort::setWorldCoords(int x, int y, bool force) {
	setWorldX(x, force);
	setWorldY(y, force);
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAScoreViewPort::setCenterCoords(int x, int y, bool force) {
	setWorldX(x - (int)(0.5*_worldW), force);
	setWorldY(y - (int)(0.5*_worldH), force);
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAScoreViewPort::setZoom(float z, int x, int y, bool force) {
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
void CAScoreViewPort::paintEvent(QPaintEvent *e) {
	if (_holdRepaint)
		return;
	
	QPainter p(this);
	int j = _drawableCList.size();
	QList<CADrawable *>* l = _drawableCList.findInRange(_worldX, _worldY, _worldW, _worldH);
	for (int i=0; i<l->size(); i++) {
		CADrawSettings s = {
	    	           _zoom,
	        	       (int)((l->at(i)->xPos() - _worldX) * _zoom),
		               (int)((l->at(i)->yPos() - _worldY) * _zoom),
	            	   drawableWidth(), drawableHeight(),
		               ((_currentContext == l->at(i))?Qt::blue:Qt::black)
		};
		l->at(i)->draw(&p, s);
	}

	
	p.drawLine(0, 0, drawableWidth(), drawableHeight());
	l = _drawableMList.findInRange(_worldX, _worldY, _worldW, _worldH);
	if (!l) return;

	for (int i=0; i<l->size(); i++) {
		CADrawSettings s = {
		               _zoom,
		               (int)((l->at(i)->xPos() - _worldX) * _zoom),
		               (int)((l->at(i)->yPos() - _worldY) * _zoom),
		               drawableWidth(), drawableHeight(),
		               (_selection.contains(l->at(i))?Qt::red:Qt::black)
		               };
		l->at(i)->draw(&p, s);
	}
	
	//flush the oldWorld coordinates as they're needed for the first repaint only
	_oldWorldX = _worldX; _oldWorldY = _worldY;
	_oldWorldW = _worldW; _oldWorldH = _worldH;
	
	delete l;	
}

void CAScoreViewPort::resizeEvent(QResizeEvent *e) {
	//ugly hack for rounding - always add 0.5, when cutting down to int, the effect is the same as it was rounded
	setWorldWidth( (int)(drawableWidth() / _zoom + 0.5) );
	setWorldHeight( (int)(drawableHeight() / _zoom + 0.5) );
	
	//setWorld methods already check for scrollbars
}

void CAScoreViewPort::checkScrollBars() {
	if ((_scrollBarsVisible != ScrollBarShowIfNeeded) || (_checkScrollBarsDeadLock))
		return;
	
	bool change = false;
	_holdRepaint = true;	//disable repaint until the scrollbar values are set
	_checkScrollBarsDeadLock = true;	//disable any further method calls until the method is over
	if ((((_drawableMList.getMaxX() > _drawableCList.getMaxX())?_drawableMList.getMaxX():_drawableCList.getMaxX()) - worldWidth() > 0) || (_hScrollBar->value()!=0)) { //if scrollbar is needed
		if (!_hScrollBar->isVisible()) {
			_hScrollBar->show();
			change = true;
		}
	} else //if the whole scene can be drawn on the canvas and the scrollbars are at position 0
		if (_hScrollBar->isVisible()) {
			_hScrollBar->hide();
			change = true;
		}
		
	if ((((_drawableMList.getMaxY() > _drawableCList.getMaxY())?_drawableMList.getMaxY():_drawableCList.getMaxY()) - worldHeight() > 0) || (_vScrollBar->value()!=0)) { //if scrollbar is needed
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

void CAScoreViewPort::processMousePressEvent(QMouseEvent *e) {
	emit CAMousePressEvent(e, QPoint((int)(e->x() / _zoom) + _worldX, (int)(e->y() / _zoom) + _worldY), this);
}

void CAScoreViewPort::processWheelEvent(QWheelEvent *e) {
	emit CAWheelEvent(e, QPoint((int)(e->x() / _zoom) + _worldX, (int)(e->y() / _zoom) + _worldY), this);	
}

void CAScoreViewPort::setScrollBarsVisibility(char status) {
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

void CAScoreViewPort::processHScrollBarEvent(int val) {
	if ((_allowManualScroll) && (!_hScrollBarDeadLock)) {
		setWorldX(val);
		repaint();
	}
}

void CAScoreViewPort::processVScrollBarEvent(int val) {
	if ((_allowManualScroll) && (!_vScrollBarDeadLock)) {
		setWorldY(val);
		repaint();
	}
}
