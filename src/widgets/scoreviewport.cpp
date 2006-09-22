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
#include <QTimer>

#include <math.h>	//neded for animated scrolls/zoom

#include <iostream>

#include "widgets/scoreviewport.h"
#include "drawable/drawable.h"
#include "drawable/drawablecontext.h"
#include "drawable/drawablemuselement.h"
#include "drawable/drawablestaff.h"
#include "drawable/drawablenote.h"
#include "core/muselement.h"
#include "core/context.h"
#include "interface/engraver.h"
#include "core/staff.h"
#include "core/note.h"

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

	//setup animation stuff
	_animationTimer = new QTimer();
	_animationTimer->setInterval(50);
	connect(_animationTimer, SIGNAL(timeout()), this, SLOT(on__animationTimer_timeout()));
	
	//set the shadow note
	_shadowNoteVisible = false;
	_shadowNoteVisibleOnLeave = false;
	
	//setup the virtual canvas
	_canvas = new QWidget(this);
	setMouseTracking(true);
	_canvas->setMouseTracking(true);
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
	_layout->setSpacing(2);
	_drawBorder = false;
	_layout->addWidget(_canvas, 0, 0);
	_layout->addWidget(_vScrollBar, 0, 1);
	_layout->addWidget(_hScrollBar, 1, 0);
	
	_oldWorldW = 0; _oldWorldH = 0;
	
}

CAScoreViewPort::~CAScoreViewPort() {
	_drawableMList.clear(true);	//clears all the elements and delete its contents too as autoDelete is true
	_drawableCList.clear(true);	//clears all the elements and delete its contents too as autoDelete is true
	
	_animationTimer->disconnect();
	_animationTimer->stop();
	delete _animationTimer;
	
	_hScrollBar->disconnect();
	_vScrollBar->disconnect();
	this->disconnect();
}

void CAScoreViewPort::on__animationTimer_timeout() {
#define ANIMATION_STEPS 7
	_animationStep++;
	
	float newZoom = _zoom + (_targetZoom - _zoom) * sqrt(((float)_animationStep)/ANIMATION_STEPS);
	int newWorldX = (int)(_worldX + (_targetWorldX - _worldX) * sqrt(((float)_animationStep)/ANIMATION_STEPS));
	int newWorldY = (int)(_worldY + (_targetWorldY - _worldY) * sqrt(((float)_animationStep)/ANIMATION_STEPS));
	int newWorldW = (int)(drawableWidth() / newZoom);
	int newWorldH = (int)(drawableHeight() / newZoom);
	
	setWorldCoords(newWorldX, newWorldY, newWorldW, newWorldH);
	
	if (_animationStep==ANIMATION_STEPS)
		_animationTimer->stop();
	
	repaint();
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
		case CADrawableMusElement::DrawableKeySignature:
			((CADrawableStaff*)elt->drawableContext())->addKeySignature((CADrawableKeySignature*)elt);
			break;
	}
}

void CAScoreViewPort::addCElement(CADrawableContext *elt, bool select) {
	_drawableCList.addElement(elt);
	
	if (select)
		setCurrentContext(elt);

	if (elt->drawableContextType() == CADrawableContext::DrawableStaff) {
		_shadowNote << new CANote(CANote::Whole, 0, 0, 0, 0);
		_shadowNote.back()->setVoice(((CADrawableStaff*)elt)->staff()->voiceAt(0));
		_shadowDrawableNote << new CADrawableNote(_shadowNote.back(), 0, 0, 0, true);
		_shadowDrawableNote.back()->setDrawableContext(elt);
	}
}

CADrawableContext *CAScoreViewPort::selectContext(CAContext *context) {
	if (!context) {
		setCurrentContext(0);
		return false;
	}
	
	for (int i=0; i<_drawableCList.size(); i++) {
		CAContext *c = ((CADrawableContext*)_drawableCList.at(i))->context();
		if (c == context) {
			setCurrentContext((CADrawableContext*)_drawableCList.at(i));
			return (CADrawableContext*)_drawableCList.at(i);
		}
	}
	
	return 0;
}

void CAScoreViewPort::setCurrentContext(CADrawableContext *drawableContext) {
	_currentContext = drawableContext;
}

CADrawableContext* CAScoreViewPort::selectCElement(int x, int y) {
	QList<CADrawable *>* l = _drawableCList.findInRange(x,y);
	
	if (l->size()!=0) {
		setCurrentContext((CADrawableContext*)l->front());
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
		if (_selection.size() != 0) {
			_selection.clear();
		}
		delete l;
		
		return 0;
	}
}

CADrawableMusElement* CAScoreViewPort::selectMElement(CAMusElement *elt) {
	_selection.clear();
	
	for (int i=0; i<_drawableMList.size(); i++)
		if (((CADrawableMusElement*)(_drawableMList.at(i)))->musElement() == elt) {
			_selection << (CADrawableMusElement*)_drawableMList.at(i);
			return (CADrawableMusElement*)_drawableMList.at(i);
		}
	
	return 0;	
}

CAMusElement *CAScoreViewPort::removeMElement(int x, int y) {
	CADrawableMusElement *elt = (CADrawableMusElement*)_drawableMList.removeElement(x,y,false);
	if (elt) {
		if (elt->drawableMusElementType() == CADrawableMusElement::DrawableClef)
			((CADrawableStaff*)elt->drawableContext())->removeClef((CADrawableClef*)elt);
		else if (elt->drawableMusElementType() == CADrawableMusElement::DrawableKeySignature)
			((CADrawableStaff*)elt->drawableContext())->removeKeySignature((CADrawableKeySignature*)elt);
		else if (elt->drawableMusElementType() == CADrawableMusElement::DrawableTimeSignature)
			((CADrawableStaff*)elt->drawableContext())->removeTimeSignature((CADrawableTimeSignature*)elt);
				
		CAMusElement *mElt = elt->musElement();
		delete elt;	//delete drawable instance

		return mElt;
	}
	
	return 0;
}

void CAScoreViewPort::importMElements(CAKDTree *elts) {
	for (int i=0; i<elts->size(); i++)
		addMElement((CADrawableMusElement*)elts->at(i)->clone());
}

void CAScoreViewPort::importCElements(CAKDTree *elts) {
	for (int i=0; i<elts->size(); i++)
		addCElement((CADrawableContext*)elts->at(i)->clone());
}

CADrawableMusElement *CAScoreViewPort::nearestLeftElement(int x, int y, bool currentContext) {
	CADrawableMusElement *elt;
	return ( (elt = (CADrawableMusElement*)_drawableMList.findNearestLeft(x, true, _currentContext))?
	         elt : 0);	//if found, return its musElement() pointer, otherwise return 0
}

CADrawableMusElement *CAScoreViewPort::nearestRightElement(int x, int y, bool currentContext) {
	CADrawableMusElement *elt;
	return ( (elt = (CADrawableMusElement*)_drawableMList.findNearestRight(x, true, _currentContext))?
	         elt : 0);	//if found, return its musElement() pointer, otherwise return 0
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

void CAScoreViewPort::rebuild() {
	//clear the shadow notes
	for (int i=0; i<_shadowNote.size(); i++) {
		delete _shadowNote[i];
		delete _shadowDrawableNote[i];
	}
	_shadowNote.clear();
	_shadowDrawableNote.clear();

	QList<CAMusElement*> musElementSelection;
	for (int i=0; i<_selection.size(); i++)
		musElementSelection << _selection[i]->musElement();
	
	_selection.clear();

	_drawableMList.clear(true);
	int contextIdx = (_currentContext ? _drawableCList.list().indexOf(_currentContext) : -1);	//remember the index of last used context
	_drawableCList.clear(true);
		
	CAEngraver::reposit(this);
	if (contextIdx != -1)	//restore the last used context
		setCurrentContext((CADrawableContext*)((_drawableCList.size() > contextIdx)?_drawableCList.list().at(contextIdx):0));
	else
		setCurrentContext(0);
	
	addToSelection(musElementSelection);
	
	checkScrollBars();
	calculateShadowNoteCoords();
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAScoreViewPort::setWorldX(int x, bool animate, bool force) {
	if (!force) {
		int maxX = (_drawableMList.getMaxX() > _drawableCList.getMaxX())?_drawableMList.getMaxX() : _drawableCList.getMaxX();
		if (x > maxX - _worldW)
			x = maxX - _worldW;
		if (x < 0)
			x = 0;
	}
	
	if (animate) {
		_targetWorldX = x;
		_targetWorldY = _worldY;
		_targetZoom = _zoom;
		startAnimationTimer();
		return;
	}

	_oldWorldX = _worldX;
	_worldX = x;
	_hScrollBarDeadLock = true;
	_hScrollBar->setValue(x);
	_hScrollBarDeadLock = false;

	checkScrollBars();
	calculateShadowNoteCoords();
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAScoreViewPort::setWorldY(int y, bool animate, bool force) {
	if (!force) {
		int maxY = (_drawableMList.getMaxY() > _drawableCList.getMaxY())?_drawableMList.getMaxY() : _drawableCList.getMaxY();
		if (y > maxY - _worldH)
			y = maxY - _worldH;
		if (y < 0)
			y = 0;
	}
	
	if (animate) {
		_targetWorldX = _worldX;
		_targetWorldY = y;
		_targetZoom = _zoom;
		startAnimationTimer();
		return;
	}

	_oldWorldY = _worldY;
	_worldY = y;
	_vScrollBarDeadLock = true;
	_vScrollBar->setValue(y);
	_vScrollBarDeadLock = false;
	
	checkScrollBars();
	calculateShadowNoteCoords();
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
		_hScrollBar->setPageStep(_worldW);
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
void CAScoreViewPort::setWorldCoords(int x, int y, int w, int h, bool animate, bool force) {
	_checkScrollBarsDeadLock = true;

	if (!drawableWidth() && !drawableHeight())
		return;
	
	float scale = (float)drawableWidth() / drawableHeight();	//always keep the world rectangle area in the same scale as the actual width/height of the drawable canvas
	if (h) {	//avoid division by zero
		if ((float)w/h > scale)
			h = (int)(w / scale);
		else
			w = (int)(h * scale);
	} else
		h = (int)(w / scale);
		
	
	setWorldWidth(w, force);
	setWorldHeight(h, force);
	setWorldX(x, animate, force);
	setWorldY(y, animate, force);
	_checkScrollBarsDeadLock = false;

	checkScrollBars();
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAScoreViewPort::setWorldCoords(int x, int y, bool animate, bool force) {
	_checkScrollBarsDeadLock = true;
	setWorldX(x, animate, force);
	setWorldY(y, animate, force);
	_checkScrollBarsDeadLock = false;
	
	checkScrollBars();
}

void CAScoreViewPort::zoomToSelection(bool animate, bool force) {
	if (!_selection.size())
		return;
	
	QRect rect;
	
	rect.setX(_selection[0]->xPos()); rect.setY(_selection[0]->yPos());
	rect.setWidth(_selection[0]->width()); rect.setHeight(_selection[0]->height());
	for (int i=1; i<_selection.size(); i++) {
		if (_selection[i]->xPos() < rect.x())
			rect.setX(_selection[i]->xPos());
		if (_selection[i]->yPos() < rect.y())
			rect.setY(_selection[i]->yPos());
		if (_selection[i]->xPos() + _selection[i]->width() > rect.x() + rect.width())
			rect.setWidth(_selection[i]->xPos() + _selection[i]->width() - rect.x());
		if (_selection[i]->yPos() + _selection[i]->height() > rect.y() + rect.height())
			rect.setHeight(_selection[i]->yPos() + _selection[i]->height() - rect.y());
	}
	
	setWorldCoords(rect, animate, force);
}

void CAScoreViewPort::zoomToWidth(bool animate, bool force) {
	int maxX = (_drawableCList.getMaxX()>_drawableMList.getMaxX())?_drawableCList.getMaxX():_drawableMList.getMaxX();
	setWorldCoords(0,0,maxX,0,animate,force);
}

void CAScoreViewPort::zoomToHeight(bool animate, bool force) {
	int maxY = (_drawableCList.getMaxY()>_drawableMList.getMaxY())?_drawableCList.getMaxY():_drawableMList.getMaxY();
	setWorldCoords(0,0,0,maxY,animate,force);
}

void CAScoreViewPort::zoomToFit(bool animate, bool force) {
	int maxX = ((_drawableCList.getMaxX() > _drawableMList.getMaxX())?_drawableCList.getMaxX():_drawableMList.getMaxX()); 
	int maxY = ((_drawableCList.getMaxY() > _drawableMList.getMaxY())?_drawableCList.getMaxY():_drawableMList.getMaxY()); 
	
	setWorldCoords(0, 0, maxX, maxY, animate, force);
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAScoreViewPort::setCenterCoords(int x, int y, bool animate, bool force) {
	_checkScrollBarsDeadLock = true;
	setWorldX(x - (int)(0.5*_worldW), animate, force);
	setWorldY(y - (int)(0.5*_worldH), animate, force);
	_checkScrollBarsDeadLock = false;

	checkScrollBars();
}

/**
 * WARNING: This method doesn't repaint the widget. You have to call repaint() manually.
 */
void CAScoreViewPort::setZoom(float z, int x, int y, bool animate, bool force) {
	bool zoomOut = false;
	if (_zoom - z > 0.0)
		zoomOut = true;

	if (animate) {
		if (!zoomOut) {
			_targetWorldX = ( _worldX - (_worldW/2) + x ) / 2;
			_targetWorldY = ( _worldY - (_worldH/2) + y ) / 2;
			_targetZoom = z;
			startAnimationTimer();
			return;
		} else {
			_targetWorldX = (int)(1.5*_worldX + 0.25*_worldW - 0.5*x);
			_targetWorldY = (int)(1.5*_worldY + 0.25*_worldH - 0.5*y);
			_targetZoom = z;
			startAnimationTimer();
			return;
		}
	}

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
	calculateShadowNoteCoords();
}

/**
 * WARNING! This method is called when the virtual _canvas gets resized as well!
 */
void CAScoreViewPort::paintEvent(QPaintEvent *e) {
	if (_holdRepaint)
		return;
	
	QPainter p(this);
	if (_drawBorder) {
		p.setPen(_borderPen);
		p.drawRect(0,0,width()-1,height()-1);
	}

	p.setClipping(true);
	if (_repaintArea) {
		p.setClipRect(QRect((int)((_repaintArea->x() - _worldX)*_zoom),
		                    (int)((_repaintArea->y() - _worldY)*_zoom),
		                    (int)(_repaintArea->width()*_zoom),
		                    (int)(_repaintArea->height()*_zoom)),
		              Qt::UniteClip);
	} else {
		p.setClipRect(QRect(_canvas->x(),
		                    _canvas->y(),
		                    _canvas->width(),
		                    _canvas->height()),
		              Qt::UniteClip);
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
	
	//draw shadow note
	if (_shadowNoteVisible) {
		for (int i=0; i<_shadowDrawableNote.size(); i++) {
			CADrawSettings s = {
			               _zoom,
			               (int)((_shadowDrawableNote[i]->xPos() - _worldX) * _zoom),
			               (int)((_shadowDrawableNote[i]->yPos() - _worldY) * _zoom),
			               drawableWidth(), drawableHeight(),
			               (Qt::gray)
			               };
			_shadowDrawableNote[i]->draw(&p, s);
		}
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
	//ugly hack for rounding - always add 0.5, when cutting down to int, the effect is the same as it was rounded for positive numbers
	setWorldCoords(_worldX, _worldY, (int)(drawableWidth() / _zoom + 0.5), (int)(drawableHeight() / _zoom + 0.5));	
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

void CAScoreViewPort::calculateShadowNoteCoords() {
	if (_currentContext?(_currentContext->drawableContextType() == CADrawableContext::DrawableStaff):0) {
		int pitch = ((CADrawableStaff*)_currentContext)->calculatePitch(_xCursor, _yCursor);	//the current staff has the real pitch we need
		for (int i=0; i<_drawableCList.size(); i++) {	//apply this pitch to all shadow notes in all staffs
			if (((CADrawableContext*)_drawableCList.at(i))->drawableContextType() != CADrawableContext::DrawableStaff)
				continue;
				
			_shadowNote[i]->setPitch(pitch);
			_shadowDrawableNote[i]->setXPos(_xCursor);
			_shadowDrawableNote[i]->setYPos(((CADrawableStaff*)_drawableCList.at(i))->calculateCenterYCoord(pitch, _xCursor));
		}
	}
}

void CAScoreViewPort::mousePressEvent(QMouseEvent *e) {
	emit CAMousePressEvent(e, QPoint((int)(e->x() / _zoom) + _worldX, (int)(e->y() / _zoom) + _worldY), this);
}

void CAScoreViewPort::mouseMoveEvent(QMouseEvent *e) {
	QPoint coords((int)(e->x() / _zoom) + _worldX, (int)(e->y() / _zoom) + _worldY);
	
	_xCursor = coords.x();
	_yCursor = coords.y();
	
	if (_shadowNoteVisible) {
		calculateShadowNoteCoords();
		repaint();
	}
	
	emit CAMouseMoveEvent(e, coords, this);
}

void CAScoreViewPort::wheelEvent(QWheelEvent *e) {
	QPoint coords((int)(e->x() / _zoom) + _worldX, (int)(e->y() / _zoom) + _worldY);
	
	emit CAWheelEvent(e, coords, this);	

	_xCursor = (int)(e->x() / _zoom) + _worldX;	//TODO: _xCursor and _yCursor are still the old one. Somehow, _zoom level and _worldX/Y are not updated when emmiting CAWheel event. -Matevz 
	_yCursor = (int)(e->y() / _zoom) + _worldY;
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

void CAScoreViewPort::leaveEvent(QEvent *e) {
	_shadowNoteVisibleOnLeave = _shadowNoteVisible;
	_shadowNoteVisible = false;
	repaint();
}
		
void CAScoreViewPort::enterEvent(QEvent *e) {
	_shadowNoteVisible = _shadowNoteVisibleOnLeave;
	repaint();
}

void CAScoreViewPort::startAnimationTimer() {
	_animationTimer->stop();
	_animationStep = 0;
	_animationTimer->start();
	on__animationTimer_timeout();
}

CADrawableMusElement *CAScoreViewPort::selectNextMusElement() {
	if (_selection.isEmpty())
		return 0;
	
	CAMusElement *musElement = _selection.back()->musElement();
	musElement = musElement->context()->findNextMusElement(musElement);
	if (!musElement)
		return 0;
	
	return selectMElement(musElement);
}

CADrawableMusElement *CAScoreViewPort::selectPrevMusElement() {
	if (_selection.isEmpty())
		return 0;
	
	CAMusElement *musElement = _selection.front()->musElement();
	musElement = musElement->context()->findPrevMusElement(musElement);
	if (!musElement)
		return 0;
	
	return selectMElement(musElement);
}

CADrawableMusElement *CAScoreViewPort::selectUpMusElement() {
	///TODO: Still needs to be written. Currently, it only returns the currently selected element.
	if (_selection.isEmpty())
		return 0;
	
	return _selection.front();
}

CADrawableMusElement *CAScoreViewPort::selectDownMusElement() {
	///TODO: Still needs to be written. Currently, it only returns the currently selected element.
	if (_selection.isEmpty())
		return 0;
	
	return _selection.front();
}

CADrawableMusElement *CAScoreViewPort::addToSelection(CAMusElement *elt) {
	for (int i=0; i<_drawableMList.size(); i++) {
		if (((CADrawableMusElement*)_drawableMList.at(i))->musElement() == elt)
			_selection << (CADrawableMusElement*)_drawableMList.at(i);
	}
	
	return _selection.back();
}

void CAScoreViewPort::addToSelection(QList<CAMusElement*> &elts) {
	for (int i=0; i<_drawableMList.size(); i++) {
		for (int j=0; j<elts.size(); j++) {
			if (elts[j] == ((CADrawableMusElement*)_drawableMList.at(i))->musElement())
				_selection << (CADrawableMusElement*)_drawableMList.at(i);
		}
	}
}
