/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
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

#include <math.h>	// neded for square root in animated scrolls/zoom

#include <iostream>

#include "widgets/scoreviewport.h"
#include "drawable/drawable.h"
#include "drawable/drawablecontext.h"
#include "drawable/drawablelyricscontext.h" // syllable edit creation
#include "drawable/drawablemuselement.h"
#include "drawable/drawablestaff.h"
#include "drawable/drawablenote.h"
#include "drawable/drawableaccidental.h"

#include "interface/engraver.h"
#include "core/document.h"
#include "core/sheet.h"
#include "core/context.h"
#include "core/muselement.h"
#include "core/staff.h"
#include "core/voice.h"
#include "core/note.h"
#include "core/rest.h"
#include "core/lyricscontext.h"
#include "core/syllable.h"
#include "canorus.h"
#include "core/settings.h"

const int CAScoreViewPort::RIGHT_EXTRA_SPACE = 100;	// Gives some space after the music so you're able to insert music elements after the last element
const int CAScoreViewPort::BOTTOM_EXTRA_SPACE = 30; // Gives some space after the music so you're able to insert new contexts below the last context
const int CAScoreViewPort::ANIMATION_STEPS = 7;

/*!
	\class CASyllableEdit
	\brief A syllable edit widget based on QLineEdit
	
	This widget is an extended QLineEdit with custom actions on keypress events - caught and determined by the main window
	then. A new signal CAKeyPressEvent() was introduced for this.
	
	Widget is usually used for editing the lyrics syllables. When left/right cursors are hit at the beginning/end of the
	line, the next syllable in the score should be selected. This isn't possible to achieve with standard QLineEdit as we
	need to control KeyPressEvent.
*/
	
CASyllableEdit::CASyllableEdit( QWidget *parent )
 : QLineEdit( parent ) {
}

CASyllableEdit::~CASyllableEdit() {
}

void CASyllableEdit::keyPressEvent( QKeyEvent *e ) {
	int oldCurPos = cursorPosition();
	QLineEdit::keyPressEvent(e); // call parent's keyPressEvent()
	
	if ((e->key()==Qt::Key_Left || e->key()==Qt::Key_Backspace || e->key()==Qt::Key_Right) && cursorPosition()!=oldCurPos)
		return;
	
	emit CAKeyPressEvent(e, this);
}

/*!
	\class CAScoreViewPort
	Widget for rendering the score.
	
	This class represents the widget capable of rendering the score. It is usually used as a central widget in the main window, but might be used
	for example, in a list of possible harmonization or improvization solutions, score layout window and more as an independent widget or only as
	a drawable content on a button.
	
	Score viewport consists of a virtual workspace full of drawable elements which represent the abstract music elements or contexts. Every drawable
	element has its absolute X, Y, Width and Height geometry properties. Score viewport also shows usually a small part of the whole workspace
	(depending on scroll bars values and a zoom level). When drawable elements are drawn, their CADrawable::draw() methods are called with the
	viewports X and Y coordinates, zoom level, pen color and other properties needed to correctly draw an element to the score viewport's canvas.
	
	Note that this widget is only capable of correctly rendering the drawable elements. No Control part of the MVC model is implemented here. The
	viewport logic is implemented outside of this class (usually main window). Score viewport only provides various signals to communicate with
	outer world. However, this class provides various modes (eg. drawing the shadow notes when inserting music elements, coloring only one voice,
	hiding certain staffs, animating the scroll etc.) the controller might use.
	
	This widget also provides horizontal and vertical scrollbars (see _hScrollBar and _vScrollBar).
*/

CAScoreViewPort::CAScoreViewPort( CASheet *sheet, QWidget *parent )
 : CAViewPort(parent) {
	initScoreViewPort( sheet );
}

CAScoreViewPort::CAScoreViewPort( QWidget *parent )
 : CAViewPort(parent) {
	initScoreViewPort( 0 );	
}

void CAScoreViewPort::initScoreViewPort( CASheet *sheet ) {
	setViewPortType( ScoreViewPort );
	
	setSheet( sheet );
	_worldX = _worldY = 0;
	_worldW = _worldH = 0;
	_zoom = 1.0;
	_holdRepaint = false;
	_hScrollBarDeadLock = false;
	_vScrollBarDeadLock = false;
	_checkScrollBarsDeadLock = false;
	_playing = false;
	_currentContext = 0;
	
	// init layout
	_layout = new QGridLayout(this);
	_layout->setMargin(2);
	_layout->setSpacing(2);
	_drawBorder = false;
	
	// init virtual canvas
	_canvas = new QWidget(this);
	setMouseTracking(true);
	_canvas->setMouseTracking(true);
	_repaintArea = 0;
	
	// init animation stuff
	_animationTimer = new QTimer();
	_animationTimer->setInterval(50);
	connect(_animationTimer, SIGNAL(timeout()), this, SLOT(on__animationTimer_timeout()));
	
	// init helpers
	setSelectedVoice( 0 );
	setShadowNoteVisible( false );
	setShadowNoteVisibleOnLeave( false );
	setShadowNoteAccs( 0 );
	setDrawShadowNoteAccs( false );
	setSyllableEdit( new CASyllableEdit( _canvas ) );
	setSyllableEditVisible( false );
	
	// init scrollbars
	_vScrollBar = new QScrollBar(Qt::Vertical, this);
	_hScrollBar = new QScrollBar(Qt::Horizontal, this);
	_vScrollBar->setMinimum(0);
	_hScrollBar->setMinimum(0);
	_vScrollBar->setTracking(true); // trigger valueChanged() when dragging the slider, not only releasing it
	_hScrollBar->setTracking(true);
	_vScrollBar->hide();
	_hScrollBar->hide();
	_scrollBarVisible = ScrollBarShowIfNeeded;
	_allowManualScroll = true;
	
	connect(_hScrollBar, SIGNAL(valueChanged(int)), this, SLOT(HScrollBarEvent(int)));
	connect(_vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(VScrollBarEvent(int)));
	
	// connect layout and widgets
	_layout->addWidget(_canvas, 0, 0);
	_layout->addWidget(_vScrollBar, 0, 1);
	_layout->addWidget(_hScrollBar, 1, 0);
	
	_oldWorldW = 0; _oldWorldH = 0;
	
	setBackgroundColor( CACanorus::settings()->backgroundColor() );
	setForegroundColor( CACanorus::settings()->foregroundColor() );
	setSelectionColor( CACanorus::settings()->selectionColor() );
	setSelectionAreaColor( CACanorus::settings()->selectionAreaColor() );
	setSelectedContextColor( CACanorus::settings()->selectedContextColor() );
	setHiddenElementsColor( CACanorus::settings()->hiddenElementsColor() );
	setDisabledElementsColor( CACanorus::settings()->disabledElementsColor() );
}

CAScoreViewPort::~CAScoreViewPort() {
	_drawableMList.clear(true);	//clears all the elements and delete its drawable contents too as autoDelete is true
	_drawableCList.clear(true);	//clears all the elements and delete its drawable contents too as autoDelete is true
	
	_animationTimer->disconnect();
	_animationTimer->stop();
	delete _animationTimer;
	
	_hScrollBar->disconnect();
	_vScrollBar->disconnect();
	this->disconnect();
}

void CAScoreViewPort::on__animationTimer_timeout() {
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
	CAScoreViewPort *v = new CAScoreViewPort(_sheet, static_cast<QWidget*>(parent()));
	
	v->importElements(&_drawableMList, &_drawableCList);
	
	return v;
}

CAScoreViewPort *CAScoreViewPort::clone(QWidget *parent) {
	CAScoreViewPort *v = new CAScoreViewPort(_sheet, parent);
	
	v->importElements(&_drawableMList, &_drawableCList);
	
	return v;
}

/*!
	Adds a drawable music element \a elt to the score viewport and selects it, if \a select is true.
*/

void CAScoreViewPort::addMElement(CADrawableMusElement *elt, bool select) {
	_drawableMList.addElement(elt);
	if (select) {
		_selection.clear();
		addToSelection(elt);
	}
	
	elt->drawableContext()->addMElement(elt);
	emit selectionChanged();
}

/*!
	Adds a drawable music element \a elt to the score viewport and selects it, if \a select is true.
*/
void CAScoreViewPort::addCElement(CADrawableContext *elt, bool select) {
	_drawableCList.addElement(elt);
	
	if (select)
		setCurrentContext(elt);
	
	if (elt->drawableContextType() == CADrawableContext::DrawableStaff) {
		_shadowNote << new CANote(CANote::Whole, 0, 0, 0, 0);
		//_shadowNote.back()->setVoice(((CADrawableStaff*)elt)->staff()->voiceAt(0));
		_shadowDrawableNote << new CADrawableNote(_shadowNote.back(), 0, 0, 0, true);
		_shadowDrawableNote.back()->setDrawableContext(elt);
	}
}

/*!
	Selects the drawable context of the given abstract context.
	If there are multiple drawable elements representing a single abstract element, selects the first one.
	
	Returns a pointer to the drawable instance of the given context or 0 if the context was not found.
	
	\sa selectMElement(CAMusElement*)
*/
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

/*!
	Selects a drawable context element at the given coordinates, if it exists.
	Returns a pointer to its abstract context element.
	If multiple elements exist at the same coordinates, they are selected one by another if you click at the same coordinates multiple times.
	If no elements are present at the coordinates, clear the selection.
*/
CADrawableContext* CAScoreViewPort::selectCElement(int x, int y) {
	QList<CADrawableContext*> l = _drawableCList.findInRange(x,y);
	
	if (l.size()!=0) {
		setCurrentContext(l.front());
	} else
		setCurrentContext(0);
	
	return currentContext();
}

/*!
	Returns a list of pointer to the drawable music elements at the given coordinates.
	If multiple elements exist at the same coordinates, they are selected one by another if you click at the same coordinates multiple times.
	If no elements are present at the coordinates, clear the selection.
*/
QList<CADrawableMusElement*> CAScoreViewPort::musElementsAt(int x, int y) {
	QList<CADrawableMusElement *> l = _drawableMList.findInRange(x,y);
	for (int i=0; i<l.size(); i++)
		if (!(l[i])->isSelectable())
			l.removeAt(i--);
	
	return l;
}

/*!
	Selects the drawable music element of the given abstract music element.
	If there are multiple drawable elements representing a single abstract element, selects the first one.
	
	Returns a pointer to the drawable instance of the given music element or 0 if the music element was not found.
	
	\sa selectCElement(CAContext*)
*/
CADrawableMusElement* CAScoreViewPort::selectMElement(CAMusElement *elt) {
	_selection.clear();
	
	for (int i=0; i<_drawableMList.size(); i++)
		if ( _drawableMList.at(i)->musElement() == elt ) {
			addToSelection(_drawableMList.at(i));
			return _drawableMList.at(i);
		}
	
	emit selectionChanged();
	return 0;	
}

/*!
	Removes a drawable music element at the given coordinates \a x and \a y, if it exists.
	Returns the pointer of the abstract music element, if the element was found and deleted.
	\warning This function only deletes the CADrawable part of the object. You still need to delete the abstract part (the pointer returned)!
*/
CAMusElement *CAScoreViewPort::removeMElement(int x, int y) {
	CADrawableMusElement *elt = _drawableMList.removeElement(x,y,false);
	if (elt) {
		if (elt->drawableMusElementType() == CADrawableMusElement::DrawableClef)
			((CADrawableStaff*)elt->drawableContext())->removeClef((CADrawableClef*)elt);
		else if (elt->drawableMusElementType() == CADrawableMusElement::DrawableKeySignature)
			((CADrawableStaff*)elt->drawableContext())->removeKeySignature((CADrawableKeySignature*)elt);
		else if (elt->drawableMusElementType() == CADrawableMusElement::DrawableTimeSignature)
			((CADrawableStaff*)elt->drawableContext())->removeTimeSignature((CADrawableTimeSignature*)elt);
		
		elt->drawableContext()->removeMElement(elt);
		CAMusElement *mElt = elt->musElement();
		delete elt;	//delete drawable instance

		return mElt;
	}
	
	return 0;
}

void CAScoreViewPort::importElements(CAKDTree<CADrawableMusElement*> *drawableMList, CAKDTree<CADrawableContext*> *drawableCList)
{
	for (int i=0; i<drawableCList->size(); i++)
		addCElement(((CADrawableContext*)drawableCList->at(i))->clone());
	for (int i=0; i<drawableMList->size(); i++)
	{
		CADrawableContext* target;
		int idx = drawableCList->list().indexOf(((CADrawableMusElement*)drawableMList->at(i))->drawableContext());
		if(idx == -1)
		{
			printf("Error!! Music element %p is not in its context!\n", drawableMList->at(i));
			target = 0;
		} else 
			target = (CADrawableContext*)_drawableCList.at(idx);
		addMElement(((CADrawableMusElement*)drawableMList->at(i))->clone(target));
	}
}

void CAScoreViewPort::importMElements(CAKDTree<CADrawableMusElement*> *elts) {
	for (int i=0; i<elts->size(); i++)
		addMElement((CADrawableMusElement*)elts->at(i)->clone());
}

void CAScoreViewPort::importCElements(CAKDTree<CADrawableContext*> *elts) {
	for (int i=0; i<elts->size(); i++)
		addCElement((CADrawableContext*)elts->at(i)->clone());
}

/*!
	Returns a pointer to the nearest drawable music element left of the current coordinates with the largest startTime.
	Drawable elements left borders are taken into account. 
	Returns the nearest element in the current context only, if currentContext is true (default).
*/
CADrawableMusElement *CAScoreViewPort::nearestLeftElement(int x, int y, bool currentContext) {
	CADrawableMusElement *elt;
	return ( (elt = _drawableMList.findNearestLeft(x, true, currentContext?_currentContext:0))?
	         elt : 0);
}

/*!
	Returns a pointer to the nearest drawable music element left of the current coordinates with the
	largest startTime in the given voice.
	Drawable elements left borders are taken into account. 
*/
CADrawableMusElement *CAScoreViewPort::nearestLeftElement(int x, int y, CAVoice *voice) {
	CADrawableMusElement *elt;
	return ( (elt = _drawableMList.findNearestLeft(x, true, 0, voice))?
	         elt : 0);
}

/*!
	Returns a pointer to the nearest drawable music element right of the current coordinates with the largest startTime.
	Drawable elements left borders are taken into account. 
	Returns the nearest element in the current context only, if currentContext is true (default).
*/
CADrawableMusElement *CAScoreViewPort::nearestRightElement(int x, int y, bool currentContext) {
	CADrawableMusElement *elt;
	return ( (elt = _drawableMList.findNearestRight(x, true, currentContext?_currentContext:0))?
	         elt : 0);
}

/*!
	Returns a pointer to the nearest drawable music element right of the current coordinates with the
	largest startTime in the given voice.
	Drawable elements left borders are taken into account. 
*/
CADrawableMusElement *CAScoreViewPort::nearestRightElement(int x, int y, CAVoice *voice) {
	CADrawableMusElement *elt;
	return ( (elt = _drawableMList.findNearestRight(x, true, 0, voice))?
	         elt : 0);
}

/*!
	Returns a pointer to the nearest upper drawable context from the given coordinates.
	\todo Also look at X coordinate
*/
CADrawableContext *CAScoreViewPort::nearestUpContext(int x, int y) {
	return static_cast<CADrawableContext*>(_drawableCList.findNearestUp(y));
}

/*!
	Returns a pointer to the nearest upper drawable context from the given coordinates.
	\todo Also look at X coordinate
*/
CADrawableContext *CAScoreViewPort::nearestDownContext(int x, int y) {
	return static_cast<CADrawableContext*>(_drawableCList.findNearestDown(y));
}

/*!
	Calculates the logical time at the given coordinates \a x and \a y.
*/
int CAScoreViewPort::calculateTime(int x, int y) {
	CADrawableMusElement *left = _drawableMList.findNearestLeft(x, true);
	CADrawableMusElement *right = _drawableMList.findNearestRight(x, true);
	
	if (left)	//the user clicked right of the element - return the nearest left element end time
		return left->musElement()->timeStart() + left->musElement()->timeLength();
	else if (right)	//the user clicked left of the element - return the nearest right element start time
		return right->musElement()->timeStart();
	else	//no elements found in the score at all - return 0
		return 0;
}

/*!
	If the given coordinates hit any of the contexts, returns that context.
*/
CAContext *CAScoreViewPort::contextCollision(int x, int y) {
	QList<CADrawableContext*> l = _drawableCList.findInRange(x, y, 0, 0);
	if (l.size() == 0) {
		return 0;
	} else {
		CAContext *context = l.front()->context(); 
		return context;
	}
}

void CAScoreViewPort::rebuild() {
	// clear the shadow notes
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
	int contextIdx = (_currentContext ? _drawableCList.list().indexOf(_currentContext) : -1);	// remember the index of last used context
	_drawableCList.clear(true);
		
	CAEngraver::reposit(this);
	
	if (contextIdx != -1)	// restore the last used context
		setCurrentContext((CADrawableContext*)((_drawableCList.size() > contextIdx)?_drawableCList.list().at(contextIdx):0));
	else
		setCurrentContext(0);
	
	addToSelection(musElementSelection);
	
	checkScrollBars();
	updateHelpers();
}

/*!
	Sets the world Top-Left X coordinate of the viewport. Animates the scroll, if \a animate is True.
	If \a force is True, sets the value despite the potential illegal value (like negative coordinates).
	
	\warning Repaint is not done automatically!
*/
void CAScoreViewPort::setWorldX(int x, bool animate, bool force) {
	if (!force) {
		int maxX = (getMaxXExtended(_drawableMList) > getMaxXExtended(_drawableCList))?getMaxXExtended(_drawableMList) : getMaxXExtended(_drawableCList);
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
	updateHelpers();
}

/*!
	Sets the world Top-Left Y coordinate of the viewport. Animates the scroll, if \a animate is True.
	If \a force is True, sets the value despite the potential illegal value (like negative coordinates).
	
	\warning Repaint is not done automatically!
*/
void CAScoreViewPort::setWorldY(int y, bool animate, bool force) {
	if (!force) {
		int maxY = getMaxYExtended(_drawableMList) > getMaxYExtended(_drawableCList)?getMaxYExtended(_drawableMList) : getMaxYExtended(_drawableCList);
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
	updateHelpers();
}

/*!
	Sets the world width of the viewport.
	If \a force is True, sets the value despite the potential illegal value (like negative coordinates).
	
	\warning Repaint is not done automatically!
*/
void CAScoreViewPort::setWorldWidth(int w, bool force) {
	if (!force) {
		if (w < 1) return;
	}
	
	_oldWorldW = _worldW;
	_worldW = w;
	
	int scrollMax;
	if ((scrollMax = ((getMaxXExtended(_drawableMList) > getMaxXExtended(_drawableCList))?getMaxXExtended(_drawableMList):getMaxXExtended(_drawableCList)) - _worldW) >= 0) {
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

/*!
	Sets the world height of the viewport.
	If \a force is True, sets the value despite the potential illegal value (like negative coordinates).
	
	\warning Repaint is not done automatically!
*/
void CAScoreViewPort::setWorldHeight(int h, bool force) {
	if (!force) {
		if (h < 1) return;
	}
	
	_oldWorldH = _worldH;
	_worldH = h;

	int scrollMax;
	if ((scrollMax = ((getMaxYExtended(_drawableMList) > getMaxYExtended(_drawableCList))?getMaxYExtended(_drawableMList):getMaxYExtended(_drawableCList)) - _worldH) >= 0) {
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

/*!
	Sets the world coordinates of the viewport to the given rectangle \a coords.
	This is an overloaded member function, provided for convenience.
	
	\warning Repaint is not done automatically!
*/
void CAScoreViewPort::setWorldCoords(QRect coords, bool animate, bool force) {
	_checkScrollBarsDeadLock = true;

	if (!drawableWidth() && !drawableHeight())
		return;
	
	float scale = (float)drawableWidth() / drawableHeight();	//always keep the world rectangle area in the same scale as the actual width/height of the drawable canvas
	if (coords.height()) {	//avoid division by zero
		if ((float)coords.width() / coords.height() > scale)
			coords.setHeight( qRound(coords.width() / scale) );
		else
			coords.setWidth( qRound(coords.height() * scale) );
	} else
		coords.setHeight( qRound(coords.width() / scale) );
		
	
	setWorldWidth(coords.width(), force);
	setWorldHeight(coords.height(), force);
	setWorldX(coords.x(), animate, force);
	setWorldY(coords.y(), animate, force);
	_checkScrollBarsDeadLock = false;

	checkScrollBars();
}

/*!
	\fn void CAScoreViewPort::setWorldCoords(int x, int y, int w, int h, bool animate, bool force)
	Sets the world coordinates of the viewport.
	This is an overloaded member function, provided for convenience.
	
	\warning Repaint is not done automatically!
	
	\param x Top-left X coordinate of the new viewport area in absolute world units.
	\param y Top-left Y coordinate of the new viewport area in absolute world units.
	\param w Width of the new viewport area in absolute world units.
	\param h Height of the new viewport area in absolute world units.
	\param animate Use animated scroll.
	\param force Use the given world units despite their illegal values (like negative coordinates etc.).
*/

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
	int maxX = (getMaxXExtended(_drawableCList)>getMaxXExtended(_drawableMList))?getMaxXExtended(_drawableCList):getMaxXExtended(_drawableMList);
	setWorldCoords(0,0,maxX,0,animate,force);
}

void CAScoreViewPort::zoomToHeight(bool animate, bool force) {
	int maxY = (getMaxYExtended(_drawableCList)>getMaxYExtended(_drawableMList))?getMaxYExtended(_drawableCList):getMaxYExtended(_drawableMList);
	setWorldCoords(0,0,0,maxY,animate,force);
}

void CAScoreViewPort::zoomToFit(bool animate, bool force) {
	int maxX = ((_drawableCList.getMaxX() > _drawableMList.getMaxX())?_drawableCList.getMaxX():_drawableMList.getMaxX()); 
	int maxY = ((_drawableCList.getMaxY() > _drawableMList.getMaxY())?_drawableCList.getMaxY():_drawableMList.getMaxY()); 
	
	setWorldCoords(0, 0, maxX, maxY, animate, force);
}

/*!
	Sets the world coordinates of the viewport, so the given coordinates are the center of the new viewport area.
	If the area has for eg. negative top-left coordinates, the area is moved to the (0,0) coordinates if \a force is False.
	ViewPort's width and height stay intact.
	\warning Repaint is not done automatically!	
*/
void CAScoreViewPort::setCenterCoords(int x, int y, bool animate, bool force) {
	_checkScrollBarsDeadLock = true;
	setWorldX(x - (int)(0.5*_worldW), animate, force);
	setWorldY(y - (int)(0.5*_worldH), animate, force);
	_checkScrollBarsDeadLock = false;

	checkScrollBars();
}

/*!
	Zooms to the given level to given direction.
	\warning Repaint is not done automatically, if \a animate is False!
	
	\param z Zoom level. (1.0 = 100%, 1.5 = 150% etc.)
	\param x X coordinate of the point of the zoom direction. 
	\param y Y coordinate of the point of the zoom direction.
	\param animate Use smooth animated zoom.
	\param force Use the given world units despite their illegal values (like negative coordinates etc.).
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
	updateHelpers();
}

/*!
	\fn void CAScoreViewPort::setZoom(float z, QPoint p, bool animate, bool force);
	Zooms to the given level to given direction.
	This is an overloaded member function, provided for convenience.
	\warning Repaint is not done automatically, if \a animate is False!
	
	\param z Zoom level. (1.0 = 100%, 1.5 = 150% etc.)
	\param p QPoint of the zoom direction.
	\param animate Use smooth animated zoom.
	\param force Use the given world units despite their illegal values (like negative coordinates etc.).
*/
		
/*!
	General Qt's paint event.
	All the music elements get actually rendered in this method.
*/
void CAScoreViewPort::paintEvent(QPaintEvent *e) {
	if (_holdRepaint)
		return;
	
	// draw the border
	QPainter p(this);
	if (_drawBorder) {
		p.setPen(_borderPen);
		p.drawRect(0,0,width()-1,height()-1);
	}

	p.setClipping(true);
	if (_repaintArea) {
		p.setClipRect(QRect(qRound((_repaintArea->x() - _worldX)*_zoom),
		                    qRound((_repaintArea->y() - _worldY)*_zoom),
		                    qRound(_repaintArea->width()*_zoom),
		                    qRound(_repaintArea->height()*_zoom)),
		              Qt::UniteClip);
	} else {
		p.setClipRect(QRect(_canvas->x(),
		                    _canvas->y(),
		                    _canvas->width(),
		                    _canvas->height()),
		              Qt::UniteClip);
	}
	
	
	// draw the background
	if (_repaintArea)
		p.fillRect(qRound((_repaintArea->x() - _worldX)*_zoom), qRound((_repaintArea->y() - _worldY)*_zoom), qRound(_repaintArea->width()*_zoom), qRound(_repaintArea->height()*_zoom), _backgroundColor);
	else
		p.fillRect(_canvas->x(), _canvas->y(), _canvas->width(), _canvas->height(), _backgroundColor);
	
	// draw contexts
	QList<CADrawableContext*> cList;
	int j = _drawableCList.size();
	if (_repaintArea)
		cList = _drawableCList.findInRange(_repaintArea->x(), _repaintArea->y(), _repaintArea->width(),_repaintArea->height());
	else
		cList = _drawableCList.findInRange(_worldX, _worldY, _worldW, _worldH);
	
	for (int i=0; i<cList.size(); i++) {
		CADrawSettings s = {
	    	           _zoom,
	        	       qRound((cList[i]->xPos() - _worldX) * _zoom),
		               qRound((cList[i]->yPos() - _worldY) * _zoom),
	            	   drawableWidth(), drawableHeight(),
		               ((_currentContext == cList[i])?selectedContextColor():foregroundColor())
		};
		cList[i]->draw(&p, s);
	}

	// draw music elements
	QList<CADrawableMusElement*> mList;
	if (_repaintArea)
		mList = _drawableMList.findInRange(_repaintArea->x(), _repaintArea->y(), _repaintArea->width(),_repaintArea->height());
	else
		mList = _drawableMList.findInRange(_worldX, _worldY, _worldW, _worldH);

	for (int i=0; i<mList.size(); i++) {
		QColor color;
		CAMusElement *elt = mList[i]->musElement();
		
		if ( _selection.contains(mList[i])) {
			color = selectionColor();
		} else
		if ( selectedVoice() &&
		     (elt &&
		      (elt->isPlayable() && static_cast<CAPlayable*>(elt)->voice()==selectedVoice() ||
		       (!elt->isPlayable() && elt->context()==selectedVoice()->staff()) ||
		       elt->context()!=selectedVoice()->staff()) ||
		      !elt && mList[i]->drawableContext()->context()==selectedVoice()->staff()
		     ) ||
		     (!selectedVoice())
		   ) {
			if ( elt && elt->musElementType()==CAMusElement::Rest &&
			     static_cast<CAPlayable*>(elt)->voice()==selectedVoice() &&
			     static_cast<CARest*>(elt)->restType()==CARest::Hidden
			   ) {
			   	color = hiddenElementsColor();
			} else if ( elt && elt->musElementType()==CAMusElement::Rest &&
			            static_cast<CARest*>(elt)->restType()==CARest::Hidden
			          ) {
			   	color = QColor(0,0,0,0); // transparent color
			} else {
				color = foregroundColor();
			}
		} else {
			if ( elt && elt->musElementType()==CAMusElement::Rest &&
			     static_cast<CARest*>(elt)->restType()==CARest::Hidden
			   ) {
			   	color = QColor(0,0,0,0); // transparent color
			} else {
				color = disabledElementsColor();
			}
		}
		
		CADrawSettings s = {
		               _zoom,
		               qRound((mList[i]->xPos() - _worldX) * _zoom),
		               qRound((mList[i]->yPos() - _worldY) * _zoom),
		               drawableWidth(), drawableHeight(),
		               color
		               };
		mList[i]->draw(&p, s);
	}
	
	// draw selection regions
	for (int i=0; i<selectionRegionList().size(); i++) {
		CADrawSettings c = {
			_zoom,
			qRound( (selectionRegionList().at(i).x() - _worldX) * _zoom),
			qRound( (selectionRegionList().at(i).y() - _worldY) * _zoom),
			qRound( selectionRegionList().at(i).width() * _zoom),
			qRound( selectionRegionList().at(i).height() * _zoom),
			selectionAreaColor()
		};
		drawSelectionRegion( &p, c );
	}
	
	// draw shadow note
	if (_shadowNoteVisible) {
		for (int i=0; i<_shadowDrawableNote.size(); i++) {
			CADrawSettings s = {
			               _zoom,
			               qRound((_shadowDrawableNote[i]->xPos() - _worldX - _shadowDrawableNote[i]->width()/2) * _zoom),
			               qRound((_shadowDrawableNote[i]->yPos() - _worldY) * _zoom),
			               drawableWidth(), drawableHeight(),
			               disabledElementsColor()
			               };
			_shadowDrawableNote[i]->draw(&p, s);
			if (_drawShadowNoteAccs) {
				CADrawableAccidental acc(_shadowNoteAccs, 0, 0, 0, _shadowDrawableNote[i]->yCenter());
				s.x -= qRound((acc.width()+2)*_zoom);
				s.y = qRound((acc.yPos() - _worldY)*_zoom);
				acc.draw(&p, s);
			}
		}
	}
	
	// flush the oldWorld coordinates as they're needed for the first repaint only
	_oldWorldX = _worldX; _oldWorldY = _worldY;
	_oldWorldW = _worldW; _oldWorldH = _worldH;
	
	if (_repaintArea) {
		delete _repaintArea;
		_repaintArea = 0;
		p.setClipping(false);
	}
}

void CAScoreViewPort::updateHelpers() {
	// Shadow notes
	if (currentContext()?(currentContext()->drawableContextType() == CADrawableContext::DrawableStaff):0) {
		int pitch = (static_cast<CADrawableStaff*>(currentContext()))->calculatePitch(_xCursor, _yCursor);	// the current staff has the real pitch we need
		for (int i=0; i<_shadowNote.size(); i++) {	// apply this pitch to all shadow notes in all staffs
			CAClef *clef = (static_cast<CADrawableStaff*>(_shadowDrawableNote[i]->drawableContext()))->getClef( _xCursor );
			_shadowNote[i]->setPitch(pitch);
			_shadowNote[i]->setNotePosition( pitch + (clef?clef->c1():-2) - 28 );
			_shadowDrawableNote[i]->setXPos(_xCursor);
			_shadowDrawableNote[i]->setYPos(
				static_cast<CADrawableStaff*>(_shadowDrawableNote[i]->drawableContext())->calculateCenterYCoord(pitch, _xCursor)
			);
		}
	}
	
	// Syllable edit
	if ( syllableEditVisible() ) {
		syllableEdit()->setFont( QFont("Century Schoolbook L", qRound(zoom()*(12-2))) );
		syllableEdit()->setGeometry(
			qRound( (syllableEditGeometry().x()-worldX())*zoom() ),
			qRound( (syllableEditGeometry().y()-worldY())*zoom() ),
			qRound( syllableEditGeometry().width()*zoom() ),
			qRound( syllableEditGeometry().height()*zoom() )
		);
		syllableEdit()->show();
	} else {
		syllableEdit()->hide();
	}
}

void CAScoreViewPort::drawSelectionRegion( QPainter *p, CADrawSettings s ) {
	p->fillRect(s.x, s.y, s.w, s.h, QBrush(s.color));
}

/*!
	Draws the border with the given pen style, color, width and other pen settings.
	Enables border.
*/
void CAScoreViewPort::setBorder(const QPen pen) {
	_borderPen = pen;
	_drawBorder = true;
}

/*!
	Disables the border.
*/
void CAScoreViewPort::unsetBorder() {
	_drawBorder = false;
}

/*!
	Called when the user resizes the widget.
	Note that repaint() event is also triggered when the internal drawable canvas changes its size (for eg. when scrollbars are shown/hidden) and the size of the viewport does not change.
*/
void CAScoreViewPort::resizeEvent(QResizeEvent *e) {
	setWorldCoords( _worldX, _worldY, qRound(drawableWidth() / _zoom), qRound(drawableHeight() / _zoom) );	
	// setWorld methods already check for scrollbars
}

/*!
	Checks whether the scrollbars are needed (the whole scene is not rendered) or not.
	Scrollbars get shown or hidden here.
	Repaint is done automatically, if needed.
*/
void CAScoreViewPort::checkScrollBars() {
	if ((isScrollBarVisible() != ScrollBarShowIfNeeded) || (_checkScrollBarsDeadLock))
		return;
	
	bool change = false;
	_holdRepaint = true;	// disable repaint until the scrollbar values are set
	_checkScrollBarsDeadLock = true;	// disable any further method calls until the method is over
	if ((((getMaxXExtended(_drawableMList) > getMaxXExtended(_drawableCList))?getMaxXExtended(_drawableMList):getMaxXExtended(_drawableCList)) - worldWidth() > 0) || (_hScrollBar->value()!=0)) { //if scrollbar is needed
		if (!_hScrollBar->isVisible()) {
			_hScrollBar->show();
			change = true;
		}
	} else // if the whole scene can be drawn on the canvas and the scrollbars are at position 0
		if (_hScrollBar->isVisible()) {
			_hScrollBar->hide();
			change = true;
		}
	
	if ((((getMaxYExtended(_drawableMList) > getMaxYExtended(_drawableCList))?getMaxYExtended(_drawableMList):getMaxYExtended(_drawableCList)) - worldHeight() > 0) || (_vScrollBar->value()!=0)) { //if scrollbar is needed
		if (!_vScrollBar->isVisible()) {
			_vScrollBar->show();
			change = true;
		}
	} else // if the whole scene can be drawn on the canvas and the scrollbars are at position 0
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

/*!
	Processes the mousePressEvent().
	A new signal is emitted: CAMousePressEvent(), which usually gets processed by the parent class then.
*/
void CAScoreViewPort::mousePressEvent(QMouseEvent *e) {
	QPoint p(qRound(e->x() / _zoom) + _worldX, qRound(e->y() / _zoom) + _worldY);
	setLastMousePressCoords(p);
	emit CAMousePressEvent(e, p, this);
}

/*!
	Processes the mouseReleaseEvent().
	A new signal is emitted: CAMouseReleaseEvent(), which usually gets processed by the parent class then.
*/
void CAScoreViewPort::mouseReleaseEvent(QMouseEvent *e) {
	emit CAMouseReleaseEvent(e, QPoint(qRound(e->x() / _zoom) + _worldX, qRound(e->y() / _zoom) + _worldY), this);
}

/*!
	Processes the mouseMoveEvent().
	A new signal is emitted: CAMouseMoveEvent(), which usually gets processed by the parent class then.
*/
void CAScoreViewPort::mouseMoveEvent(QMouseEvent *e) {
	QPoint coords((int)(e->x() / _zoom) + _worldX, (int)(e->y() / _zoom) + _worldY);
	
	_xCursor = coords.x();
	_yCursor = coords.y();
	
	emit CAMouseMoveEvent(e, coords, this);

	if (_shadowNoteVisible) {
		updateHelpers();
	}
}

/*!
	Processes the wheelEvent().
	A new signal is emitted: CAWheelEvent(), which usually gets processed by the parent class then.
*/
void CAScoreViewPort::wheelEvent(QWheelEvent *e) {
	QPoint coords((int)(e->x() / _zoom) + _worldX, (int)(e->y() / _zoom) + _worldY);
	
	emit CAWheelEvent(e, coords, this);	

	_xCursor = (int)(e->x() / _zoom) + _worldX;	//TODO: _xCursor and _yCursor are still the old one. Somehow, _zoom level and _worldX/Y are not updated when emmiting CAWheel event. -Matevz 
	_yCursor = (int)(e->y() / _zoom) + _worldY;
}

/*!
	Processes the keyPressEvent().
	A new signal is emitted: CAKeyPressEvent(), which usually gets processed by the parent class then.
*/
void CAScoreViewPort::keyPressEvent(QKeyEvent *e) {
	emit CAKeyPressEvent(e, this);
}

void CAScoreViewPort::setScrollBarVisible(CAScrollBarVisibility status) {
	_scrollBarVisible = status;
	
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

/*!
	Processes the Horizontal scroll bar event.
	This method is called when the horizontal scrollbar changes its value, let it be internally or due to user interaction.
*/
void CAScoreViewPort::HScrollBarEvent(int val) {
	if ((_allowManualScroll) && (!_hScrollBarDeadLock)) {
		setWorldX(val);
		repaint();
	}
}

/*!
	Processes the Vertical scroll bar event.
	This method is called when the horizontal scrollbar changes its value, let it be internally or due to user interaction.
*/
void CAScoreViewPort::VScrollBarEvent(int val) {
	if ((_allowManualScroll) && (!_vScrollBarDeadLock)) {
		setWorldY(val);
		repaint();
	}
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

/*!
	Selects the next music element in the current context or appends the next music element to the selection if \a append is True.
	Returns a pointer to the newly selected drawable music element or 0, if such an element doesn't exist or the selection is empty.
	
	This method is usually called when using the right arrow key.
*/
CADrawableMusElement *CAScoreViewPort::selectNextMusElement( bool append ) {
	if (_selection.isEmpty())
		return 0;
	
	CAMusElement *musElement = _selection.back()->musElement();
	musElement = musElement->context()->findNextMusElement(musElement);
	if (!musElement)
		return 0;
	
	if (append)
		return addToSelection( musElement );
	else
		return selectMElement( musElement );
}

/*!
	Selects the previous music element in the current context or appends the previous music element to the selection if \a append is True.
	Returns a pointer to the newly selected drawable music element or 0, if such an element doesn't exist or the selection is empty.
	
	This method is usually called when using the left arrow key.
*/
CADrawableMusElement *CAScoreViewPort::selectPrevMusElement( bool append ) {
	if (_selection.isEmpty())
		return 0;
	
	CAMusElement *musElement = _selection.front()->musElement();
	musElement = musElement->context()->findPrevMusElement(musElement);
	if (!musElement)
		return 0;
	
	if (append)
		return addToSelection( musElement );
	else
		return selectMElement( musElement );
}

/*!
	Selects the upper music element in the current context.
	Returns a pointer to the newly selected drawable music element or 0, if such an element doesn't exist or the selection is empty.
	
	This method is usually called when using the up arrow key.
	\todo Still needs to be written. Currently, it only returns the currently selected element.
*/
CADrawableMusElement *CAScoreViewPort::selectUpMusElement() {
	if (_selection.isEmpty())
		return 0;
	
	return _selection.front();
}

/*!
	Selects the lower music element in the current context.
	Returns a pointer to the newly selected drawable music element or 0, if such an element doesn't exist or the selection is empty.
	
	This method is usually called when using the up arrow key.
	\todo Still needs to be written. Currently, it only returns the currently selected element.
*/
CADrawableMusElement *CAScoreViewPort::selectDownMusElement() {
	if (_selection.isEmpty())
		return 0;
	
	return _selection.front();
}

/*!
	Adds the given drawable music element \a elt to the current selection.
*/
void CAScoreViewPort::addToSelection( CADrawableMusElement *elt, bool triggerSignal ) {
	int i;
	for (i=0; i<_selection.size() && _selection[i]->xPos() < elt->xPos(); i++);
	_selection.insert( i, elt );
	
	if ( triggerSignal )
		emit selectionChanged();
}

/*!
	Adds the given list of drawable music elements \a list to the current selection.
*/
void CAScoreViewPort::addToSelection(const QList<CADrawableMusElement*> list, bool selectableOnly ) {
	for (int i=0; i<list.size(); i++) {
		if ( selectableOnly && list[i]->isSelectable() )
			addToSelection(list[i], false);
	}
	
	emit selectionChanged();
}

/*!
	Adds the drawable music element of the given abstract music element \a elt to the selection.
	Returns a pointer to its drawable element or 0, if the music element is not part of this score viewport.
*/
CADrawableMusElement *CAScoreViewPort::addToSelection(CAMusElement *elt) {
	for (int i=0; i<_drawableMList.size(); i++) {
		if ( static_cast<CADrawableMusElement*>(_drawableMList.at(i))->musElement() == elt )
			addToSelection(static_cast<CADrawableMusElement*>(_drawableMList.at(i)));
	}
	
	emit selectionChanged();
	return _selection.back();
}

/*!
	Adds the given list of abstract music elements to the selection.
*/
void CAScoreViewPort::addToSelection(const QList<CAMusElement*> elts) {
	for (int i=0; i<_drawableMList.size(); i++) {
		for (int j=0; j<elts.size(); j++) {
			if ( elts[j] == static_cast<CADrawableMusElement*>(_drawableMList.at(i))->musElement() )
				addToSelection(static_cast<CADrawableMusElement*>(_drawableMList.at(i)));
		}
	}
	emit selectionChanged();
}

/*!
	Finds the drawable instance of the given abstract music element.
	
	\sa findCElement()
*/
CADrawableMusElement *CAScoreViewPort::findMElement(CAMusElement *elt) {
	for (int i=0; i<_drawableMList.size(); i++)
		if ( static_cast<CADrawableMusElement*>(_drawableMList.at(i))->musElement()==elt )
			return static_cast<CADrawableMusElement*>(_drawableMList.at(i));
}	
	
/*!
	Finds the drawable instance of the given abstract context.
	
	\sa findMElement()
*/
CADrawableContext *CAScoreViewPort::findCElement(CAContext *context) {
	for (int i=0; i<_drawableCList.size(); i++)
		if (static_cast<CADrawableContext*>(_drawableCList.at(i))->context()==context)
			return static_cast<CADrawableContext*>(_drawableCList.at(i));
}

/*!
	Creates a CASyllableEdit widget over the existing drawable syllable \a dMusElt.
	Returns the pointer to the created widget.
	
	\sa createSyllableEdit( QRect geometry )
*/
CASyllableEdit *CAScoreViewPort::createSyllableEdit( CADrawableMusElement *dMusElt ) {
	if (!dMusElt || dMusElt->drawableMusElementType()!=CADrawableMusElement::DrawableSyllable)
		return 0;
	
	CADrawableLyricsContext *dlc = static_cast<CADrawableLyricsContext*>(dMusElt->drawableContext());
	CASyllable *syllable = static_cast<CASyllable*>(dMusElt->musElement());
	if (!dlc || !syllable) return 0;
	
	int xPos=dMusElt->xPos(), yPos=dlc->yPos(), width=100, height=dlc->height();
	
	CADrawableMusElement *dRight = findMElement( dlc->lyricsContext()->findNextMusElement( syllable ) );
	if (dRight)
		width = dRight->xPos() - dMusElt->xPos();
	
	QString text = syllable->text();
	if (syllable->hyphenStart()) text+="-";
	else if (syllable->melismaStart()) text+="_";
	syllableEdit()->setText(text);
	setSyllableEditVisible( true );
	setSyllableEditGeometry( QRect(xPos-2, yPos, width+2, height) );
	updateHelpers(); // show it
	syllableEdit()->setFocus();
	
	return syllableEdit();
}

/*!
	Removes and deletes the line edit for creating a lyrics syllable.
*/
void CAScoreViewPort::removeSyllableEdit() {
	setSyllableEditVisible( false ); // don't delete it, just hide it!
	updateHelpers();
	syllableEdit()->setText("");
	this->setFocus();
}
	
/*!
	Returns the maximum X of the viewable World a little bigger to make insertion at the end easy.
*/
template <typename T>
int CAScoreViewPort::getMaxXExtended(CAKDTree<T> &v) {
	return v.getMaxX() + RIGHT_EXTRA_SPACE;
}

/*!
	Returns the maximum Y of the viewable World a little bigger to make insertion at the end easy.
*/
template <typename T>
int CAScoreViewPort::getMaxYExtended(CAKDTree<T> &v) {
	return v.getMaxY() + BOTTOM_EXTRA_SPACE;
}

/*!
	Returns a list of drawable contexts the current score viewport includes between
	the vertical coordinates \a y1 and \a y2.
	The context is in a list already if only part of the context is touched by the region.
	That is the first returned context's top border is smaller than \a y1 and the last returned context's
	bottom border is larger than \a x2.
*/
QList<CADrawableContext*> CAScoreViewPort::findContextsInRegion( QRect &region ) {
	return _drawableCList.findInRange(region);
}

/*!
	\fn CASheet *CAScoreViewPort::sheet()
	Returns the pointer to the viewport's sheet it represents.
*/

/*!
	\fn bool CAScoreViewPort::removeFromSelection(CADrawableMusElement *elt)
	Removes the given drawable music element \a elt from the selection, if it exists.
	Returns True, if element existed in the selection and was removed, false otherwise.
*/

/*!
	\fn void CAScoreViewPort::clearSelection()
	Clears the current selection. Its behaviour is the same as calling clearMSelection() and clearCSelection().
*/

/*!
	\fn QList<CAMusElement*> CAScoreViewPort::selection()
	Returns a list of the currently selected drawable music elements.
*/

/*!
	\var bool CAScoreViewPort::_allowManualScroll
	This property holds whether a user interaction with the scrollbars actually triggers the scroll of the viewport.
*/

/*!
	\enum CAScoreViewPort::CAScrollBarVisibility
	Different behaviour of the scroll bars:
		- ScrollBarAlwaysVisible - scrollbars are always visible, no matter if the whole scene can be rendered on canvas or not
		- ScrollBarAlwaysHidden - scrollbars are always hidden, no matter if the whole scene can be rendered on canvas or not
		- ScrollBarShowIfNeeded - scrollbars are visible, if they are needed (the current viewport area is too small to render the whole
		  scene), otherwise hidden. This is default behaviour.
*/

/*!
	\fn float CAScoreViewPort::zoom()
	Returns the zoom level of the viewport (1.0 = 100%, 1.5 = 150% etc.).
*/

/*!
	\fn void CAScoreViewPort::setRepaintArea(QRect *area)
	Sets the area to be repainted, not the whole widget.
	
	\sa clearRepaintArea()
*/ 

/*!
	\fn void CAScoreViewPort::clearRepaintArea()
	Disables and deletes the area to be repainted.
	
	\sa setRepaintArea()
*/

/*!
	\fn void CAScoreViewPort::CAMousePressEvent(QMouseEvent *e, QPoint p, CAScoreViewPort *v)
	
	This signal is emitted when mousePressEvent() is called. Parent class is usually connected to this event.
	It adds another two arguments to the mousePressEvent() function - pointer to this viewport and coordinates
	in world coordinates where user used the mouse.
	This is useful when a parent class wants to know which class the signal was emmitted by.
	
	\param e Mouse event which gets processed.
	\param p Coordinates of the mouse cursor in absolute world values.
	\param v Pointer to this viewport (the viewport which emmitted the signal).
*/

/*!
	\fn void CAScoreViewPort::CAMouseMoveEvent(QMouseEvent *e, QPoint p, CAScoreViewPort *v)
	
	This signal is emitted when mouseMoveEvent() is called. Parent class is usually connected to this event.
	It adds another two arguments to the mouseMoveEvent() function - pointer to this viewport and coordinates
	in world coordinates where user used the mouse.
	This is useful when a parent class wants to know which class the signal was emmitted by.
	
	\param e Mouse event which gets processed.
	\param p Coordinates of the mouse cursor in absolute world values.
	\param v Pointer to this viewport (the viewport which emmitted the signal).
*/

/*!
	\fn void CAScoreViewPort::CAMouseReleaseEvent(QMouseEvent *e, QPoint p, CAScoreViewPort *v)
	
	This signal is emitted when mouseReleaseEvent() is called. Parent class is usually connected to this event.
	It adds another two arguments to the mouseReleaseEvent() function - pointer to this score viewport and coordinates
	in world coordinates where user used the mouse.
	This is useful when a parent class wants to know which class the signal was emmitted by.
	
	\param e Mouse event which gets processed.
	\param p Coordinates of the mouse cursor in absolute world values.
	\param v Pointer to this viewport (the viewport which emmitted the signal).
*/

/*!
	\fn void CAScoreViewPort::CAWheelEvent(QWheelEvent *e, QPoint p, CAScoreViewPort *v)
	
	This signal is emitted when wheelEvent() is called. Parent class is usually connected to this event.
	It adds another two arguments to the wheelEvent() function - pointer to this score viewport and coordinates
	in world coordinates where user used the mouse.
	This is useful when a parent class wants to know which class the signal was emmitted by.
	
	\param e Wheel event which gets processed.
	\param p Coordinates of the mouse cursor in absolute world values.
	\param v Pointer to this viewport (the viewport which emmitted the signal).
*/

/*!
	\fn void CAScoreViewPort::CAKeyPressEvent(QKeyEvent *e, CAScoreViewPort *v)
	
	This signal is emitted when keyPressEvent() is called. Parent class is usually connected to this event.
	It adds another two arguments to the wheelEvent() function - pointer to this score viewport.
	This is useful when a parent class wants to know which class the signal was emmitted by.
	
	\param e Wheel event which gets processed.
	\param v Pointer to this viewport (the viewport which emmitted the signal).
*/
