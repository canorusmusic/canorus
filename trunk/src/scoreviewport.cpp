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
#include <QBrush>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPalette>
#include <QColor>

#include <iostream>

#include "scoreviewport.h"
#include "drawable.h"
#include "drawablecontext.h"
#include "drawablemuselement.h"
#include "drawablestaff.h"
#include "drawablenote.h"
#include "muselement.h"
#include "context.h"
#include "engraver.h"
#include "staff.h"

CAScoreViewPort::CAScoreViewPort(CASheet *sheet, QWidget *parent) : CAViewPort(parent) {
	_viewPortType = CAViewPort::ScoreViewPort;
	
	_sheet = sheet;
	_worldX = _worldY = 0;
	_zoom = 1.0;
	_holdRepaint = false;
	_hScrollBarDeadLock = false;
	_vScrollBarDeadLock = false;
	_checkScrollBarsDeadLock = false;
	_playing = false;
	
	_currentContext = 0;
	
	//setup the virtual canvas
	_canvas = new QWidget(this);
	_backgroundBrush = QBrush(QColor(255, 255, 240));
	_repaintArea = 0;
	
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
	
	connect(_hScrollBar, SIGNAL(valueChanged(int)), this, SLOT(HScrollBarEvent(int)));
	connect(_vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(VScrollBarEvent(int)));
	
	//setup layout
	_layout = new QGridLayout(this);
	_layout->setMargin(2);
	_drawBorder = false;
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

void CAScoreViewPort::addMElement(CADrawableMusElement *elt, bool select) {
	_drawableMList.addElement(elt);
	if (select) {
		_selection.clear();
		_selection << elt;
	}
	
	switch (elt->drawableMusElementType()) {
		case CADrawableMusElement::DrawableClef:
			((CADrawableStaff*)elt->drawableContext())->addClef((CADrawableClef*)elt);
			break;
	}
}

void CAScoreViewPort::addCElement(CADrawableContext *elt, bool select) {
	_drawableCList.addElement(elt);
	if (select)
		_currentContext = elt;
}

bool CAScoreViewPort::selectContext(CAContext *context) {
	if (!context) {
		_currentContext = 0;
		return false;
	}
	
	for (int i=0; i<_drawableCList.size(); i++) {
		CAContext *c=((CADrawableContext*)_drawableCList.at(i))->context();
		if (c == context) {
			_currentContext = (CADrawableContext*)_drawableCList.at(i);
			return true;
		}
	}
	
	return false;
}

CADrawableContext* CAScoreViewPort::selectCElement(int x, int y) {
	QList<CADrawable *>* l = _drawableCList.findInRange(x,y);
	
	if (l->size()!=0) {
		_currentContext = (CADrawableContext*)l->front();
	}
	
	delete l;
	
	return (_currentContext?_currentContext:0);
}

CAMusElement* CAScoreViewPort::selectMElement(int x, int y) {
	QList<CADrawable *>* l;
	if ((l=_drawableMList.findInRange(x,y))->size() != 0) { //multiple elements can share the same coordinates
		int idx;
			_selection.clear();
		if ( (_selection.size() != 1) || ((idx = l->indexOf(_selection.front())) == -1) ) {
			_selection << (CADrawableMusElement*)l->at(0);	//if the previous selection was not a single element or if the new list doesn't contain the selection, add the first element in the available list to the selection
		} else {
			_selection << (CADrawableMusElement*)l->at((++idx < l->size()) ? idx : 0); //if there are two or more elements with the same coordinates, select the next one (behind it). This way, you can click multiple times on the same place and you'll always select the other element.
		}
		
		delete l;

		return ((CADrawableMusElement*)_selection.front())->musElement();
	} else {
		_selection.clear();
		delete l;
		
		return 0;
	}
}

bool CAScoreViewPort::selectMElement(CAMusElement *elt) {
	_selection.clear();
	
	for (int i=0; i<_drawableMList.size(); i++)
		if (((CADrawableMusElement*)(_drawableMList.at(i)))->musElement() == elt) {
			_selection << (CADrawableMusElement*)_drawableMList.at(i);
			return true;
		}
	
	return false;	
}

CAMusElement *CAScoreViewPort::removeMElement(int x, int y) {
	CADrawableMusElement *elt = (CADrawableMusElement*)_drawableMList.removeElement(x,y,false);
	if (elt) {
		CAMusElement *mElt = elt->musElement();
		delete elt;
		return mElt;
	}
	
	return 0;
}

void CAScoreViewPort::importMElements(CAKDTree *elts) {
	_drawableMList.import(elts);
}

void CAScoreViewPort::importCElements(CAKDTree *elts) {
	_drawableCList.import(elts);
}

CAMusElement *CAScoreViewPort::nearestLeftElement(int x, int y, bool currentContext) {
	CADrawableMusElement *elt;
	return ( (elt = (CADrawableMusElement*)_drawableMList.findNearestLeft(x, true, _currentContext))?
	         elt->musElement() : 0);	//if found, return its musElement() pointer, otherwise return 0
}

CAMusElement *CAScoreViewPort::nearestRightElement(int x, int y, bool currentContext) {
	CADrawableMusElement *elt;
	return ( (elt = (CADrawableMusElement*)_drawableMList.findNearestRight(x, true, _currentContext))?
	         elt->musElement() : 0);	//if found, return its musElement() pointer, otherwise return 0
}

int CAScoreViewPort::calculateTime(int x, int y) {
	CADrawableMusElement *left = (CADrawableMusElement *)_drawableMList.findNearestLeft(x, true);
	CADrawableMusElement *right = (CADrawableMusElement *)_drawableMList.findNearestRight(x, true);
	
	if (left)	//the user clicked right of the element - return the nearest left element end time
		return left->musElement()->timeStart() + left->musElement()->timeLength();
	else if (right)	//the user clicked left of the element - return the nearest right element start time
		return right->musElement()->timeStart();
	else	//no elements found in the score at all - return 0
		return 0;
}

CAContext *CAScoreViewPort::contextCollision(int x, int y) {
	QList<CADrawable*> *l;
	if ((l = _drawableCList.findInRange(x, y, 0, 0))->size() == 0) {
		delete l;
		return 0;
	} else {
		CAContext *context = ((CADrawableContext*)l->front())->context(); 
		delete l;
		return context;
	}
}

void CAScoreViewPort::update() {
	_drawableMList.clear(true);
	int contextIdx = (_currentContext ? _drawableCList.list().indexOf(_currentContext) : -1);	//remember the index of last used context
	_drawableCList.clear(true);
	_selection.clear();
	
	CAEngraver::reposit(this);
	if (contextIdx != -1)	//restore the last used context
		_currentContext = (CADrawableContext*)((_drawableCList.size() > contextIdx)?_drawableCList.list().at(contextIdx):0);
	else
		_currentContext = 0;
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
	if (_repaintArea) {
		p.setClipping(true);
		p.setClipRect(QRect((int)((_repaintArea->x() - _worldX)*_zoom),
		                    (int)((_repaintArea->y() - _worldY)*_zoom),
		                    (int)(_repaintArea->width()*_zoom),
		                    (int)(_repaintArea->height()*_zoom)),
		              Qt::UniteClip);
	}
	
	if (_drawBorder) {
		p.setPen(_borderPen);
		p.drawRect(0,0,width()-1,height()-1);
	}
	
	//draw the background
	if (_repaintArea)
		p.fillRect((int)((_repaintArea->x() - _worldX)*_zoom), (int)((_repaintArea->y() - _worldY)*_zoom), (int)(_repaintArea->width()*_zoom), (int)(_repaintArea->height()*_zoom), _backgroundBrush);
	else
		p.fillRect(_canvas->x(), _canvas->y(), _canvas->width(), _canvas->height(), _backgroundBrush);

	QList<CADrawable *>* l;
	//draw contexts
	int j = _drawableCList.size();
	if (_repaintArea)
		l = _drawableCList.findInRange(_repaintArea->x(), _repaintArea->y(), _repaintArea->width(),_repaintArea->height());
	else
		l = _drawableCList.findInRange(_worldX, _worldY, _worldW, _worldH);

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

	delete l;
	
	//draw music elements
	if (_repaintArea)
		l = _drawableMList.findInRange(_repaintArea->x(), _repaintArea->y(), _repaintArea->width(),_repaintArea->height());
	else
		l = _drawableMList.findInRange(_worldX, _worldY, _worldW, _worldH);

	for (int i=0; i<l->size(); i++) {
		CADrawSettings s = {
		               _zoom,
		               (int)((l->at(i)->xPos() - _worldX) * _zoom),
		               (int)((l->at(i)->yPos() - _worldY) * _zoom),
		               drawableWidth(), drawableHeight(),
		               (_selection.contains((CADrawableMusElement*)l->at(i))?Qt::red:Qt::black)
		               };
		l->at(i)->draw(&p, s);
	}
	
	//flush the oldWorld coordinates as they're needed for the first repaint only
	_oldWorldX = _worldX; _oldWorldY = _worldY;
	_oldWorldW = _worldW; _oldWorldH = _worldH;
	
	if (_repaintArea) {
		delete _repaintArea;
		_repaintArea = 0;
		p.setClipping(false);
	}
	delete l;
}

void CAScoreViewPort::setBorder(const QPen pen) {
	_borderPen = pen;
	_drawBorder = true;
}

void CAScoreViewPort::setBackground(const QBrush brush) {
	_backgroundBrush = brush;
}

void CAScoreViewPort::unsetBorder() {
	_drawBorder = false;
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

void CAScoreViewPort::mousePressEvent(QMouseEvent *e) {
	emit CAMousePressEvent(e, QPoint((int)(e->x() / _zoom) + _worldX, (int)(e->y() / _zoom) + _worldY), this);
}

void CAScoreViewPort::wheelEvent(QWheelEvent *e) {
	emit CAWheelEvent(e, QPoint((int)(e->x() / _zoom) + _worldX, (int)(e->y() / _zoom) + _worldY), this);	
}

void CAScoreViewPort::keyPressEvent(QKeyEvent *e) {
	emit CAKeyPressEvent(e, this);
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

void CAScoreViewPort::HScrollBarEvent(int val) {
	if ((_allowManualScroll) && (!_hScrollBarDeadLock)) {
		setWorldX(val);
		repaint();
	}
}

void CAScoreViewPort::VScrollBarEvent(int val) {
	if ((_allowManualScroll) && (!_vScrollBarDeadLock)) {
		setWorldY(val);
		repaint();
	}
}

CADrawableMusElement *CAScoreViewPort::find(CAMusElement *elt) {
	for (int i=0; i<_drawableMList.size(); i++)
		if (((CADrawableMusElement*)_drawableMList.at(i))->musElement()==elt)
			return (CADrawableMusElement*)_drawableMList.at(i);
}
