/*!
	Copyright (c) 2006-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QScrollBar>
#include <QPainter>
#include <QBrush>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPalette>
#include <QColor>
#include <QTimer>

#include <math.h>	// needed for square root in animated scrolls/zoom

#include <iostream>

#include "widgets/scoreview.h"
#include "layout/drawable.h"
#include "layout/drawablecontext.h"
#include "layout/drawablelyricscontext.h" // syllable edit creation
#include "layout/drawablemuselement.h"
#include "layout/drawablestaff.h"
#include "layout/drawablenote.h"
#include "layout/drawableaccidental.h"

#include "layout/layoutengine.h"
#include "score/document.h"
#include "score/sheet.h"
#include "score/context.h"
#include "score/muselement.h"
#include "score/staff.h"
#include "score/voice.h"
#include "score/note.h"
#include "score/rest.h"
#include "score/lyricscontext.h"
#include "score/syllable.h"
#include "score/text.h"
#include "score/bookmark.h"
#include "canorus.h"
#include "core/settings.h"

const int CAScoreView::RIGHT_EXTRA_SPACE = 100;	// Gives some space after the music so you're able to insert music elements after the last element
const int CAScoreView::BOTTOM_EXTRA_SPACE = 30; // Gives some space after the music so you're able to insert new contexts below the last context
const int CAScoreView::ANIMATION_STEPS = 7;

/*!
	\class CATextEdit
	\brief A text edit widget based on QLineEdit

	This widget is extended QLineEdit with custom actions on keypress events - caught and determined usually by the main
	window then. A new signal CAKeyPressEvent() was introduced for this.

	Widget is usually used for editing the lyrics syllables or writing other text in the score.
	For syllable editing, when left/right cursors are hit at the beginning/end of the line, the next/previous syllable in
	the score should be selected. This isn't possible to achieve with standard QLineEdit as we need to control KeyPressEvent.
	This behaviour is again determined in the main window.
*/

CATextEdit::CATextEdit( QWidget *parent )
 : QLineEdit( parent ) {
}

CATextEdit::~CATextEdit() {
}

void CATextEdit::keyPressEvent( QKeyEvent *e ) {
	int oldCurPos = cursorPosition();
	QLineEdit::keyPressEvent(e); // call parent's keyPressEvent()

	if ((e->key()==Qt::Key_Left || e->key()==Qt::Key_Backspace || e->key()==Qt::Key_Right) && cursorPosition()!=oldCurPos)
		return;

	emit CAKeyPressEvent(e);
}

/*!
	\class CAScoreView
	Widget for rendering the score.

	This class represents the widget capable of rendering the score. It is usually used as a central widget in the main window, but might be used
	for example, in a list of possible harmonization or improvization solutions, score layout window and more as an independent widget or only as
	a drawable content on a button.

	Score view consists of a virtual workspace full of drawable elements which represent the abstract music elements or contexts. Every drawable
	element has its absolute X, Y, Width and Height geometry properties. Score view also shows usually a small part of the whole workspace
	(depending on scroll bars values and a zoom level). When drawable elements are drawn, their CADrawable::draw() methods are called with the
	views X and Y coordinates, zoom level, pen color and other properties needed to correctly draw an element to the score view's canvas.

	Note that this widget is only capable of correctly rendering the drawable elements. No Control part of the MVC model is implemented here. The
	view logic is implemented outside of this class (usually main window). Score view only provides various signals to communicate with
	outer world. However, this class provides various modes (eg. drawing the shadow notes when inserting music elements, coloring only one voice,
	hiding certain staffs, animating the scroll etc.) the controller might use.

	This widget also provides horizontal and vertical scrollbars (see _hScrollBar and _vScrollBar).
*/

CAScoreView::CAScoreView( CASheet *sheet, QWidget *parent )
 : CAView(parent) {
	initScoreView( sheet );
}

CAScoreView::CAScoreView( QWidget *parent )
 : CAView(parent) {
	initScoreView( 0 );
}

void CAScoreView::initScoreView( CASheet *sheet ) {
	setViewType( ScoreView );

	setSheet( sheet );
	_playing = false;
	_currentContext = 0;
/*	_xCursor = _yCursor = 0;
*/	setResizeDirection( CADrawable::Undefined );

	// init graphics scene
	_canvas = new QGraphicsView(this);
	_scene = new QGraphicsScene();
	_canvas->setScene( _scene );

	// init layout
	_drawBorder = false;
	_grabTabKey = true;
	setFocusPolicy( Qt::StrongFocus );

	// init animation stuff
	_animationTimer = new QTimer(this);
	_animationTimer->setInterval(50);
	connect(_animationTimer, SIGNAL(timeout()), this, SLOT(on_animationTimer_timeout()));

	// init click timer (used for measuring double/triple click since Qt4 doesn't support triple click yet ;))
	_clickTimer = new QTimer(this);
	_clickTimer->setSingleShot(true);
	_clickTimer->setInterval( static_cast<int>(QApplication::doubleClickInterval()*1.5) );
	connect( _clickTimer, SIGNAL(timeout()), this, SLOT(on_clickTimer_timeout()) );

	// init helpers
	setSelectedVoice( 0 );
	setShadowNoteVisible( false );
	setShadowNoteVisibleOnLeave( false );
	setShadowNoteAccs( 0 );
	setNoteNameVisible( false );
	setDrawShadowNoteAccs( false );
	setTextEdit( new CATextEdit( _canvas ) );
	setTextEditVisible( false );

	// set default parameters
	setBackgroundBrush( CACanorus::settings()->backgroundColor() );
	setForegroundColor( CACanorus::settings()->backgroundColor() );
	setSelectionColor( CACanorus::settings()->selectionColor() );
	setSelectionAreaBrush( CACanorus::settings()->selectionAreaColor() );
	setSelectedContextColor( CACanorus::settings()->selectedContextColor() );
	setHiddenElementsColor( CACanorus::settings()->hiddenElementsColor() );
	setDisabledElementsColor( CACanorus::settings()->disabledElementsColor() );
}

CAScoreView::~CAScoreView() {
	// Delete the drawable elements/contexts
	_drawableMList.clear(true);
	_drawableCList.clear(true);

	while(!_shadowNote.isEmpty()) {
		delete _shadowNote.takeFirst();
		delete _shadowDrawableNote.takeFirst(); // same size
	}

	_animationTimer->disconnect();
	_animationTimer->stop();
	delete _animationTimer;
}

void CAScoreView::on_animationTimer_timeout() {
	_animationStep++;
	double z = zoom();
	QRectF r = sceneRect();

	double newZoom = z + (_targetZoom - z) * sqrt(((double)_animationStep)/ANIMATION_STEPS);
	double newWorldX = r.x() + (_targetWorldX - r.x()) * sqrt(((double)_animationStep)/ANIMATION_STEPS);
	double newWorldY = r.y() + (_targetWorldY - r.y()) * sqrt(((double)_animationStep)/ANIMATION_STEPS);
	double newWorldW = drawableWidth() / newZoom;
	double newWorldH = drawableHeight() / newZoom;

	setSceneRect(newWorldX, newWorldY, newWorldW, newWorldH);

	if (_animationStep==ANIMATION_STEPS)
		_animationTimer->stop();

	repaint();
}

CAScoreView *CAScoreView::clone() {
	CAScoreView *v = new CAScoreView(_sheet, static_cast<QWidget*>(parent()));

	v->importElements(&_drawableMList, &_drawableCList);

	return v;
}

CAScoreView *CAScoreView::clone(QWidget *parent) {
	CAScoreView *v = new CAScoreView(_sheet, parent);

	v->importElements(&_drawableMList, &_drawableCList);

	return v;
}

/*!
	Adds a drawable music element \a elt to the score view and selects it, if \a select is true.
*/

void CAScoreView::addMElement(CADrawableMusElement *elt, bool select) {
	_drawableMList.addElement(elt);
	if (select) {
		_selection.clear();
		addToSelection(elt);
	}

	elt->drawableContext()->addMElement(elt);
	_scene->addItem( elt );
	emit selectionChanged();
}

/*!
	Adds a drawable music element \a elt to the score view and selects it, if \a select is true.
*/
void CAScoreView::addCElement(CADrawableContext *elt, bool select) {
	_drawableCList.addElement(elt);

	if (select)
		setCurrentContext(elt);

	if (elt->drawableContextType() == CADrawableContext::DrawableStaff &&
	    static_cast<CAStaff*>(elt->context())->voiceList().size()) {
		_shadowNote << new CANote( CADiatonicPitch(), CAPlayableLength(CAPlayableLength::Quarter, 0), static_cast<CAStaff*>(elt->context())->voiceList()[0], 0 );
		_shadowDrawableNote << new CADrawableNote(_shadowNote.back(), elt, true);
	}
	_scene->addItem( elt );
}

/*!
	Selects the drawable context of the given abstract context.
	If there are multiple drawable elements representing a single abstract element, selects the first one.

	Returns a pointer to the drawable instance of the given context or 0 if the context was not found.

	\sa selectMElement(CAMusElement*)
*/
CADrawableContext *CAScoreView::selectContext(CAContext *context) {
	if (!context) {
		setCurrentContext(0);
		return 0;
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
	Change the x-coord of the last mouse press coords to after the right most element on the given list.
*/
void CAScoreView::setLastMousePressCoordsAfter(const QList<CAMusElement*> list) {
	double maxX = 0;
	for( int i=0; i < _drawableMList.size(); i++) {
		CADrawableMusElement* delt = _drawableMList.at(i);
/*		if(list.contains(delt->musElement()))
			maxX = qMax(delt->xPos() + delt->width(), maxX);
*/	}
	QPoint newCoords(lastMousePressCoords());
	newCoords.setX(maxX);
	setLastMousePressCoords(newCoords);
}

/*!
	Selects a drawable context element at the given coordinates, if it exists.
	Returns a pointer to its abstract context element.
	If multiple elements exist at the same coordinates, they are selected one by another if you click at the same coordinates multiple times.
	If no elements are present at the coordinates, clear the selection.
*/
CADrawableContext* CAScoreView::selectCElement(double x, double y) {
	QList<CADrawableContext*> l = _drawableCList.findInRange(x,y);

	if (l.size()!=0) {
		setCurrentContext(l.front());
	} else
		setCurrentContext(0);

	return currentContext();
}

/*!
	Returns a list of pointers to the drawable music elements at the given coordinates.

	Multiple elements can exist at the same coordinates.

	If there is a currently selected voice, only elements belonging to this voice are selected.
*/
QList<CADrawableMusElement*> CAScoreView::musElementsAt(double x, double y) {
	QList<CADrawableMusElement *> l = _drawableMList.findInRange(x,y);
/*	for (int i=0; i<l.size(); i++)
		if ( !l[i]->isSelectable() || selectedVoice() && l[i]->musElement() && l[i]->musElement()->isPlayable() && static_cast<CAPlayable*>(l[i]->musElement())->voice()!=selectedVoice() )
			l.removeAt(i--);
*/
	return l;
}

/*!
	Selects the drawable music element of the given abstract music element.
	If there are multiple drawable elements representing a single abstract element, selects the first one.

	Returns a pointer to the drawable instance of the given music element or 0 if the music element was not found.

	\sa selectCElement(CAContext*)
*/
CADrawableMusElement* CAScoreView::selectMElement(CAMusElement *elt) {
	_selection.clear();

	for (int i=0; i<_drawableMList.size(); i++) {
/*		if ( _drawableMList.at(i)->musElement() == elt && _drawableMList.at(i)->isSelectable() ) {
			addToSelection(_drawableMList.at(i));
		}
*/	}

	emit selectionChanged();

	if ( selection().size() )
		return selection().back();
	else
		return 0;
}

/*!
	Removes a drawable music element at the given coordinates \a x and \a y, if it exists.
	Returns the pointer of the abstract music element, if the element was found and deleted.
	\warning This function only deletes the CADrawable part of the object. You still need to delete the abstract part (the pointer returned)!
*/
CAMusElement *CAScoreView::removeMElement(double x, double y) {
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
		delete elt;	// delete drawable instance

		return mElt;
	}

	return 0;
}

void CAScoreView::importElements(CAKDTree<CADrawableMusElement*> *drawableMList, CAKDTree<CADrawableContext*> *drawableCList)
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

void CAScoreView::importMElements(CAKDTree<CADrawableMusElement*> *elts) {
	for (int i=0; i<elts->size(); i++)
		addMElement((CADrawableMusElement*)elts->at(i)->clone());
}

void CAScoreView::importCElements(CAKDTree<CADrawableContext*> *elts) {
	for (int i=0; i<elts->size(); i++)
		addCElement((CADrawableContext*)elts->at(i)->clone());
}

/*!
	Returns a pointer to the nearest drawable music element left of the current coordinates with the largest startTime.
	Drawable elements left borders are taken into account.
	If \a context is non-zero, returns the nearest element in the given context only.
*/
CADrawableMusElement *CAScoreView::nearestLeftElement(double x, double y, CADrawableContext* context) {
	return _drawableMList.findNearestLeft(x, true, context);
}

/*!
	Returns a pointer to the nearest drawable music element left of the current coordinates with the
	largest startTime in the given voice.
	Drawable elements left borders are taken into account.
*/
CADrawableMusElement *CAScoreView::nearestLeftElement(double x, double y, CAVoice *voice) {
	return _drawableMList.findNearestLeft(x, true, 0, voice);
}

/*!
	Returns a pointer to the nearest drawable music element right of the current coordinates with the largest startTime.
	Drawable elements left borders are taken into account.
	If \a context is non-zero, returns the nearest element in the given context only.
*/
CADrawableMusElement *CAScoreView::nearestRightElement(double x, double y, CADrawableContext* context) {
	return _drawableMList.findNearestRight(x, true, context);
}

/*!
	Returns a pointer to the nearest drawable music element right of the current coordinates with the
	largest startTime in the given voice.
	Drawable elements left borders are taken into account.
*/
CADrawableMusElement *CAScoreView::nearestRightElement(double x, double y, CAVoice *voice) {
	return _drawableMList.findNearestRight(x, true, 0, voice);
}

/*!
	Returns a pointer to the nearest upper drawable context from the given coordinates.
	\todo Also look at X coordinate
*/
CADrawableContext *CAScoreView::nearestUpContext(double x, double y) {
	return static_cast<CADrawableContext*>(_drawableCList.findNearestUp(y));
}

/*!
	Returns a pointer to the nearest upper drawable context from the given coordinates.
	\todo Also look at X coordinate
*/
CADrawableContext *CAScoreView::nearestDownContext(double x, double y) {
	return static_cast<CADrawableContext*>(_drawableCList.findNearestDown(y));
}

/*!
	Calculates the logical time at the given coordinates \a x and \a y.
*/
int CAScoreView::calculateTime(double x, double y) {
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
CAContext *CAScoreView::contextCollision(double x, double y) {
	QList<CADrawableContext*> l = _drawableCList.findInRange(x, y, 0, 0);
	if (l.size() == 0) {
		return 0;
	} else {
		CAContext *context = l.front()->context();
		return context;
	}
}

/*!
	Calls the engraver to reposition the music elements on the canvas.
	Also updates scrollbars.
 */
void CAScoreView::rebuild() {
	// clear the shadow notes
	CAPlayableLength l( CAPlayableLength::Quarter );
	for (int i=0; i<_shadowNote.size(); i++) {
		delete _shadowDrawableNote[i];
		l = _shadowNote[i]->playableLength();
		delete _shadowNote[i];
	}
	_shadowNote.clear();
	_shadowDrawableNote.clear();

	QList<CAMusElement*> musElementSelection;
	for (int i=0; i<_selection.size(); i++) {
		if ( !musElementSelection.contains( _selection[i]->musElement() ) )
			musElementSelection << _selection[i]->musElement();
	}

	_selection.clear();

	_drawableMList.clear(true);
	int contextIdx = (_currentContext ? _drawableCList.list().indexOf(_currentContext) : -1);	// remember the index of last used context
	_drawableCList.clear(true);
	_scene->clear();

	CALayoutEngine::reposit(this);

	for (int i=0; i<_shadowNote.size(); i++) {
		_shadowNote[i]->setPlayableLength(l);
	}

	if (contextIdx != -1)	// restore the last used context
		setCurrentContext((CADrawableContext*)((_drawableCList.size() > contextIdx)?_drawableCList.list().at(contextIdx):0));
	else
		setCurrentContext(0);

	addToSelection(musElementSelection);

	updateHelpers();
}

/*!
	Sets the world coordinates of the view to the given rectangle \a coords.
	This is an overloaded member function, provided for convenience.
*/
void CAScoreView::setSceneRect(const QRectF& coords, bool animate) {
	if (!drawableWidth() && !drawableHeight())
		return;

	// set the actual new coords
	double x = coords.x();
	double y = coords.y();
	double w = coords.width();
	double h = coords.height();

	// always keep the world rectangle area in the same scale as the actual width/height of the drawable canvas
	double scale = drawableWidth() / drawableHeight();
	if (coords.height()) {	// avoid division by zero
		if (coords.width() / coords.height() > scale) {
			h = coords.width() / scale;
		} else {
			w = coords.height() * scale;
		}
	} else {
		h = coords.width() / scale;
	}



	// check the limit for width and height
	double scrollMax;
	scrollMax = qMax(getMaxXExtended(_drawableMList), getMaxXExtended(_drawableCList));
	_canvas->horizontalScrollBar()->setMaximum(scrollMax);
	_canvas->horizontalScrollBar()->setPageStep(drawableWidth());

	scrollMax = qMax(getMaxYExtended(_drawableMList), getMaxYExtended(_drawableCList));
	_canvas->verticalScrollBar()->setMaximum(scrollMax);
	_canvas->verticalScrollBar()->setPageStep(drawableHeight());

	if (animate) {
		_targetWorldX = x;
		_targetWorldY = y;
		_targetZoom = zoom();
		startAnimationTimer();
	} else {
		_canvas->setSceneRect( QRectF( x, y, w, h) );
	}

	updateHelpers();
}

void CAScoreView::zoomToSelection(bool animate) {
	if (!_selection.size())
		return;

	QRect rect;

	rect.setX(_selection[0]->pos().x()); rect.setY(_selection[0]->pos().y());
/*	rect.setWidth(_selection[0]->width()); rect.setHeight(_selection[0]->height());
*/	for (int i=1; i<_selection.size(); i++) {
		if (_selection[i]->pos().x() < rect.x())
			rect.setX(_selection[i]->pos().x());
		if (_selection[i]->pos().y() < rect.y())
			rect.setY(_selection[i]->pos().y());
/*		if (_selection[i]->pos().x() + _selection[i]->width() > rect.x() + rect.width())
			rect.setWidth(_selection[i]->pos().x() + _selection[i]->width() - rect.x());
		if (_selection[i]->pos().y() + _selection[i]->height() > rect.y() + rect.height())
			rect.setHeight(_selection[i]->pos().y() + _selection[i]->height() - rect.y());
*/	}

	setSceneRect(rect, animate);
}

void CAScoreView::zoomToWidth(bool animate) {
	setSceneRect(0,0, qMax(getMaxXExtended(_drawableCList), getMaxXExtended(_drawableMList)), 0,animate);
}

void CAScoreView::zoomToHeight(bool animate) {
	setSceneRect(0,0,0, qMax(getMaxYExtended(_drawableCList), getMaxYExtended(_drawableMList)), animate);
}

void CAScoreView::zoomToFit(bool animate) {
	setSceneRect(0,
			     0,
			     qMax(getMaxXExtended(_drawableCList), getMaxXExtended(_drawableMList)),
	             qMax(getMaxYExtended(_drawableCList), getMaxYExtended(_drawableMList)),
	             animate);
}

/*!
	Sets the world coordinates of the view, so the given coordinates are the center of the new view area.
	View's width and height stay intact.
*/
void CAScoreView::centerOn(double x, double y, bool animate) {
	setSceneRect(x - 0.5*sceneRect().width(),
			     y - 0.5*sceneRect().height(),
			     sceneRect().width(),
			     sceneRect().height(),
			     animate);
}

/*!
	Zooms to the given level to given direction.
	\warning Repaint is not done automatically, if \a animate is False!

	\param z Zoom level. (1.0 = 100%, 1.5 = 150% etc.)
	\param x X coordinate of the point of the zoom direction.
	\param y Y coordinate of the point of the zoom direction.
	\param animate Use smooth animated zoom.
*/
void CAScoreView::setZoom(double z, double x, double y, bool animate) {
	bool zoomOut = false;
	if (zoom() - z > 0.0)
		zoomOut = true;

	QRectF rect = sceneRect();
	if (animate) {
		if (!zoomOut) {
			_targetWorldX = ( rect.x() - (rect.width()/2) + x ) / 2;
			_targetWorldY = ( rect.y() - (rect.height()/2) + y ) / 2;
			_targetZoom = z;
			startAnimationTimer();
		} else {
			_targetWorldX = 1.5*rect.x() + 0.25*rect.width() - 0.5*x;
			_targetWorldY = 1.5*rect.y() + 0.25*rect.height() - 0.5*y;
			_targetZoom = z;
			startAnimationTimer();
		}
	} else {
		if (!zoomOut) { //zoom in
			//the new view's center coordinates will become the middle point of the current view center coords and the mouse pointer coords
			centerOn( ( rect.x() + (rect.width()/2) + x ) / 2,
							 ( rect.y() + (rect.height()/2) + y ) / 2 );
		} else { //zoom out
			//the new view's center coordinates will become the middle point of the current view center coords and the mirrored over center pointer coords
			//worldX_ + (worldW_/2) + (worldX_ + (worldW_/2) - x)/2
			centerOn( 1.5*rect.x() + 0.75*rect.width() - 0.5*x,
							 1.5*rect.y() + 0.75*rect.height() - 0.5*y );
		}
		updateHelpers();
	}
}

/*!
	General Qt's paint event.
	All the music elements get actually rendered in this method.
*/
/*void CAScoreView::paintEvent(QPaintEvent *e) {
	// draw the border
	QPainter p(this);
	if (_drawBorder) {
		p.setPen(_borderPen);
		p.drawRect(0,0,width()-1,height()-1);
	}

	p.setClipping(true);
	p.setClipRect(QRect(_canvas->x(),
						_canvas->y(),
						_canvas->width(),
						_canvas->height()),
				  Qt::UniteClip);


	// draw the background
	p.fillRect(_canvas->x(), _canvas->y(), _canvas->width(), _canvas->height(), _backgroundColor);

	// draw contexts
	QList<CADrawableContext*> cList;
	int j = _drawableCList.size();
	cList = _drawableCList.findInRange(sceneRect().x(), sceneRect().y(), _worldW, _worldH);

	for (int i=0; i<cList.size(); i++) {
		CADrawSettings s = {
	    	           zoom(),
	        	       qRound((cList[i]->pos().x() - sceneRect().x()) * zoom()),
		               qRound((cList[i]->pos().y() - sceneRect().y()) * zoom()),
	            	   drawableWidth(), drawableHeight(),
		               ((_currentContext == cList[i])?selectedContextColor():foregroundColor()),
		               sceneRect().x(),
		               sceneRect().y()
		};
		cList[i]->draw(&p, s);
	}

	// draw music elements
	QList<CADrawableMusElement*> mList;
	mList = _drawableMList.findInRange(sceneRect().x(), sceneRect().y(), _worldW, _worldH);

	p.setRenderHint( QPainter::Antialiasing, CACanorus::settings()->antiAliasing() );

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
			            static_cast<CARest*>(elt)->restType()==CARest::Hidden ||
			            elt && !elt->isVisible()
			          ) {
			   	color = QColor(0,0,0,0); // transparent color
			} else if ( elt && elt->color()!=QColor(0,0,0,0) ) {
				color = elt->color(); // set elements color, if defined
			} else {
				color = foregroundColor(); // set default color for foreground elements
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
		               zoom(),
		               qRound((mList[i]->pos().x() - sceneRect().x()) * zoom()),
		               qRound((mList[i]->pos().y() - sceneRect().y()) * zoom()),
		               drawableWidth(), drawableHeight(),
		               color,
		               sceneRect().x(),
		               sceneRect().y()
		               };
		mList[i]->draw(&p, s);
		if ( _selection.contains(mList[i]) && mList[i]->isHScalable() ) {
			s.color = foregroundColor();
			mList[i]->drawHScaleHandles(&p, s);
		}
		if ( _selection.contains(mList[i]) && mList[i]->isVScalable() ) {
			s.color = foregroundColor();
			mList[i]->drawVScaleHandles(&p, s);
		}
	}

	// draw selection regions
	for (int i=0; i<selectionRegionList().size(); i++) {
		CADrawSettings c = {
			zoom(),
			qRound( (selectionRegionList().at(i).x() - sceneRect().x()) * zoom()),
			qRound( (selectionRegionList().at(i).y() - sceneRect().y()) * zoom()),
			qRound( selectionRegionList().at(i).width() * zoom()),
			qRound( selectionRegionList().at(i).height() * zoom()),
			selectionAreaColor(),
            sceneRect().x(),
            sceneRect().y()
		};
		drawSelectionRegion( &p, c );
	}

	// draw shadow note
	if (_shadowNoteVisible) {
		for (int i=0; i<_shadowDrawableNote.size(); i++) {
			if ( CACanorus::settings()->shadowNotesInOtherStaffs() || _shadowDrawableNote[i]->drawableContext() == currentContext() ) {
				CADrawSettings s = {
					zoom(),
					qRound((_shadowDrawableNote[i]->pos().x() - sceneRect().x() - _shadowDrawableNote[i]->width()/2) * zoom()),
					qRound((_shadowDrawableNote[i]->pos().y() - sceneRect().y()) * zoom()),
					drawableWidth(), drawableHeight(),
					disabledElementsColor(),
	               sceneRect().x(),
	               sceneRect().y()
				};

				_shadowDrawableNote[i]->draw(&p, s);

				if (_drawShadowNoteAccs) {
					CADrawableAccidental acc(_shadowNoteAccs, 0, 0, 0, _shadowDrawableNote[i]->yCenter());
					s.x -= qRound((acc.width()+2)*zoom());
					s.y = qRound((acc.pos().y() - sceneRect().y())*zoom());
					acc.draw(&p, s);
				}
			}
		}

		// draw note name
		if (_shadowNote.size()) {
			QFont font("FreeSans");
			font.setPixelSize( 20 );
			p.setFont(font);
			p.setPen(disabledElementsColor());
			p.drawText( qRound((_xCursor-sceneRect().x()+10) * zoom()), qRound((_yCursor-sceneRect().y()-10) * zoom()), CANote::generateNoteName(_shadowNote[0]->diatonicPitch().noteName(), _shadowNoteAccs) );
		}
	}

	// flush the oldWorld coordinates as they're needed for the first repaint only
	_oldWorldX = sceneRect().x(); _oldWorldY = sceneRect().y();
	_oldWorldW = _worldW; _oldWorldH = _worldH;
}
*/
void CAScoreView::updateHelpers() {
	// Shadow notes
	if (currentContext()?(currentContext()->drawableContextType() == CADrawableContext::DrawableStaff):0) {
		int pitch = 0;/*(static_cast<CADrawableStaff*>(currentContext()))->calculatePitch(_xCursor, _yCursor);	// the current staff has the real pitch we need
		*/for (int i=0; i<_shadowNote.size(); i++) {	// apply this pitch to all shadow notes in all staffs
			CADiatonicPitch dPitch(pitch, 0);
			_shadowNote[i]->setDiatonicPitch( dPitch );

			if (selectedVoice()) {
				_shadowNote[i]->setStemDirection( selectedVoice()->stemDirection() );
			}

			CADrawableContext *c = _shadowDrawableNote[i]->drawableContext();
/*			delete _shadowDrawableNote[i];
			_shadowDrawableNote[i] = new CADrawableNote(_shadowNote[i], c, _xCursor, static_cast<CADrawableStaff*>(c)->calculateCenterYCoord(pitch, _xCursor), true);
*/		}
	}

	// Text edit widget
	if ( textEditVisible() ) {
		textEdit()->setFont( QFont("Century Schoolbook L", qRound(zoom()*(12-2))) );
		textEdit()->setGeometry(
			qRound( (textEditGeometry().x()-sceneRect().x())*zoom() ),
			qRound( (textEditGeometry().y()-sceneRect().y())*zoom() ),
			qRound( textEditGeometry().width()*zoom() ),
			qRound( textEditGeometry().height()*zoom() )
		);
		textEdit()->show();
	} else {
		textEdit()->hide();
	}
}

/*void CAScoreView::drawSelectionRegion( QPainter *p, CADrawSettings s ) {
	p->fillRect(s.x, s.y, s.w, s.h, QBrush(s.color));
}
*/
/*!
	Draws the border with the given pen style, color, width and other pen settings.
	Enables border.
*/
void CAScoreView::setBorder(const QPen pen) {
	_borderPen = pen;
	_drawBorder = true;
}

/*!
	Disables the border.
*/
void CAScoreView::unsetBorder() {
	_drawBorder = false;
}

/*!
	Called when the user resizes the widget.
*/
void CAScoreView::resizeEvent(QResizeEvent *e) {
	_canvas->resize(e->size());
}

/*!
	This functions forward the Tab and Shift+Tab keys to keyPressEvent(), if grabTabKey is True.
*/
bool CAScoreView::event( QEvent *event ) {
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
		if ((keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab) && _grabTabKey) {
			keyPressEvent( keyEvent );
			return true;
		}
	}
	return CAView::event(event);
}

/*!
	Processes the mousePressEvent().
	A new signal is emitted: CAMousePressEvent(), which usually gets processed by the parent class then.
*/
void CAScoreView::mousePressEvent(QMouseEvent *e) {
	CAView::mousePressEvent(e);
	QPoint coords(e->x() / zoom() + sceneRect().x(), e->y() / zoom() + sceneRect().y());
/*	if ( selection().size() && selection()[0]->isHScalable() && coords.y()>=selection()[0]->pos().y() && coords.y()<=selection()[0]->pos().y()+selection()[0]->height() ) {
		if ( coords.x()==selection()[0]->pos().x()  ) {
			setResizeDirection(CADrawable::Left);
		} else if ( coords.x()==selection()[0]->pos().x()+selection()[0]->width() ) {
			setResizeDirection(CADrawable::Right);
		}
	} else if ( selection().size() && selection().at(0)->isVScalable() && coords.x()>=selection()[0]->pos().x() && coords.x()<=selection()[0]->pos().x()+selection()[0]->width() ) {
		if ( coords.y()==selection()[0]->pos().y() ) {
			setResizeDirection(CADrawable::Top);
		} else if ( coords.y()==selection()[0]->pos().y()+selection()[0]->height() ) {
			setResizeDirection(CADrawable::Bottom);
		}
	}
*/
	if ( !_clickTimer->isActive() || coords!=lastMousePressCoords() ) {
		_clickTimer->start();
	}

	emit CAMousePressEvent( e, coords );

	if ( coords!=lastMousePressCoords() ) {
		setLastMousePressCoords(coords);
		_numberOfClicks=1;
	} else {
		_numberOfClicks++;
	}

	if (_numberOfClicks==2) {
		emit CADoubleClickEvent( e, coords );
	} else
	if (_numberOfClicks==3) {
		emit CATripleClickEvent( e, coords );
	}
}

/*!
	Clears number of clicks done so far inside one interval.
	This function is usually click timer's slot.
 */
void CAScoreView::on_clickTimer_timeout() {
	_numberOfClicks = 0;
}

/*!
	Processes the mouseReleaseEvent().
	A new signal is emitted: CAMouseReleaseEvent(), which usually gets processed by the parent class then.
*/
void CAScoreView::mouseReleaseEvent(QMouseEvent *e) {
	emit CAMouseReleaseEvent(e, QPoint(e->x() / zoom() + sceneRect().x(), e->y() / zoom() + sceneRect().y()));
	setResizeDirection( CADrawable::Undefined );
}

/*!
	Processes the mouseMoveEvent().
	A new signal is emitted: CAMouseMoveEvent(), which usually gets processed by the parent class then.
*/
void CAScoreView::mouseMoveEvent(QMouseEvent *e) {
	QPoint coords(e->x() / zoom() + sceneRect().x(), e->y() / zoom() + sceneRect().y());

/*	_xCursor = coords.x();
	_yCursor = coords.y();
*/
	bool isHScalable = false;
/*	for ( int i=0; i<selection().size() && !isHScalable; i++) {
		if ( selection()[i]->isHScalable() && (coords.x()==selection()[i]->pos().x() || coords.x()==selection()[i]->pos().x()+selection()[i]->width()) &&
		     coords.y()>=selection()[i]->pos().y() && coords.y()<=selection()[i]->pos().y()+selection()[i]->height()
		   )
			isHScalable = true;
	}
*/
	bool isVScalable = false;
/*	for ( int i=0; i<selection().size() && !isVScalable; i++) {
		if ( selection()[i]->isVScalable() && (coords.y()==selection()[i]->pos().y() || coords.y()==selection()[i]->pos().y()+selection()[i]->height()) &&
		     coords.x()>=selection()[i]->pos().x() && coords.x()<=selection()[i]->pos().x()+selection()[i]->width()
		   )
			isVScalable = true;
	}
*/
	if (isHScalable)
		setCursor(Qt::SizeHorCursor);
	else if (isVScalable)
		setCursor(Qt::SizeVerCursor);
	else
		setCursor(Qt::ArrowCursor);

	emit CAMouseMoveEvent(e, coords);
}

/*!
	Processes the wheelEvent().
	A new signal is emitted: CAWheelEvent(), which usually gets processed by the parent class then.
*/
/*void CAScoreView::wheelEvent(QWheelEvent *e) {
	QPoint coords((int)(e->x() / zoom()) + sceneRect().x(), (int)(e->y() / zoom()) + sceneRect().y());

	emit CAWheelEvent(e, coords);

	_xCursor = (int)(e->x() / zoom()) + sceneRect().x();	//TODO: _xCursor and _yCursor are still the old one. Somehow, zoom() level and sceneRect().x()/Y are not updated when emmiting CAWheel event. -Matevz
	_yCursor = (int)(e->y() / zoom()) + sceneRect().y();
}
*/
/*!
	Processes the keyPressEvent().
	A new signal is emitted: CAKeyPressEvent(), which usually gets processed by the parent class then.
*/
void CAScoreView::keyPressEvent(QKeyEvent *e) {
	emit CAKeyPressEvent(e);
}

Qt::ScrollBarPolicy CAScoreView::scrollBarPolicy() {
	return _canvas->horizontalScrollBarPolicy();
}

void CAScoreView::setScrollBarPolicy(Qt::ScrollBarPolicy p) {
	_canvas->setHorizontalScrollBarPolicy( p );
	_canvas->setVerticalScrollBarPolicy( p );
}

void CAScoreView::leaveEvent(QEvent *e) {
	_shadowNoteVisibleOnLeave = _shadowNoteVisible;
	_shadowNoteVisible = false;
	repaint();
}

void CAScoreView::enterEvent(QEvent *e) {
	_shadowNoteVisible = _shadowNoteVisibleOnLeave;
	repaint();
}

void CAScoreView::startAnimationTimer() {
	_animationTimer->stop();
	_animationStep = 0;
	_animationTimer->start();
	on_animationTimer_timeout();
}

/*!
	Selects the next music element in the current context (voice, if selectedVoice is set) or appends the next music element
	to the selection, if \a append is True.
	Returns a pointer to the newly selected drawable music element or 0, if such an element doesn't exist or the selection is empty.

	This method is usually called when using the right arrow key.
*/
CADrawableMusElement *CAScoreView::selectNextMusElement( bool append ) {
	if (_selection.isEmpty())
		return 0;

	CAMusElement *musElement = _selection.back()->musElement();
	if ( selectedVoice() )
		musElement = selectedVoice()->next(musElement);
	else
		musElement = musElement->context()->next(musElement);

	if (!musElement)
		return 0;

	if (append)
		return addToSelection( musElement );
	else
		return selectMElement( musElement );
}

/*!
	Selects the next music element in the current context (voice, if selectedVoice is set) or appends the next music element
	to the selection, if \a append is True.
	Returns a pointer to the newly selected drawable music element or 0, if such an element doesn't exist or the selection is empty.

	This method is usually called when using the left arrow key.
*/
CADrawableMusElement *CAScoreView::selectPrevMusElement( bool append ) {
	if (_selection.isEmpty())
		return 0;

	CAMusElement *musElement = _selection.front()->musElement();
	if ( selectedVoice() )
		musElement = selectedVoice()->previous(musElement);
	else
		musElement = musElement->context()->previous(musElement);

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
CADrawableMusElement *CAScoreView::selectUpMusElement() {
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
CADrawableMusElement *CAScoreView::selectDownMusElement() {
	if (_selection.isEmpty())
		return 0;

	return _selection.front();
}

/*!
	Adds the given drawable music element \a elt to the current selection.
*/
void CAScoreView::addToSelection( CADrawableMusElement *elt, bool triggerSignal ) {
	int i;
	for (i=0; i<_selection.size() && _selection[i]->pos().x() < elt->pos().x(); i++);

/*	if ( elt->isSelectable() )
		_selection.insert( i, elt );
*/
	if ( triggerSignal )
		emit selectionChanged();
}

/*!
	Adds the given list of drawable music elements \a list to the current selection.
*/
void CAScoreView::addToSelection(const QList<CADrawableMusElement*> list, bool selectableOnly ) {
	for (int i=0; i<list.size(); i++) {
/*		if ( !selectableOnly || selectableOnly && list[i]->isSelectable() )
			addToSelection(list[i], false);
*/	}

	emit selectionChanged();
}

/*!
	Adds the drawable music element of the given abstract music element \a elt to the selection.
	Returns a pointer to its drawable element or 0, if the music element is not part of this score view.
*/
CADrawableMusElement *CAScoreView::addToSelection(CAMusElement *elt) {
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
void CAScoreView::addToSelection(const QList<CAMusElement*> elts) {
	for (int i=0; i<_drawableMList.size(); i++) {
/*		if ( _drawableMList.at(i)->isSelectable() ) {
			for (int j=0; j<elts.size(); j++) {
				if ( elts[j] == static_cast<CADrawableMusElement*>(_drawableMList.at(i))->musElement() )
					addToSelection(static_cast<CADrawableMusElement*>(_drawableMList.at(i)), false);
			}
		}
*/	}

	emit selectionChanged();
}

/*!
	Select all elements in the view.
	This function is usually associated with CTRL+A key.
*/
void CAScoreView::selectAll() {
	clearSelection();

	for(int i=0; i<_drawableMList.size(); i++)
		addToSelection(_drawableMList.at(i), false);

	emit selectionChanged();
}

/*!
	Selects all elements in the current bar.
	This function is usually called when double clicking on the bar.
 */
void CAScoreView::selectAllCurBar() {
	if ( !currentContext() || currentContext()->drawableContextType()!=CADrawableContext::DrawableStaff ) {
		return;
	}

	clearSelection();

	if (selectedVoice()) {
		addToSelection( selectedVoice()->getBar( coordsToTime(_lastMousePressCoords.x()) ) );
	} else {
		QList<CAVoice*> voices = static_cast<CAStaff*>(currentContext()->context())->voiceList();
		for (int i=0; i<voices.size(); i++) {
			addToSelection( voices[i]->getBar( coordsToTime(_lastMousePressCoords.x()) ) );
		}
	}

	emit selectionChanged();
}

/*!
	Selects all elements in the current context (line).
	This function is usually called when triple clicking on the context.
 */
void CAScoreView::selectAllCurContext() {
	if ( !currentContext() ) {
		return;
	}

	clearSelection();
	addToSelection( currentContext()->drawableMusElementList(), false );

	emit selectionChanged();
}

/*!
	Inverts the current selection.
*/
void CAScoreView::invertSelection() {
	QList<CADrawableMusElement *> oldSelection = selection();
	clearSelection();

	for(int i=0; i<_drawableMList.size(); i++)
		if(!oldSelection.contains(_drawableMList.at(i)))
			addToSelection(_drawableMList.at(i), false);

	emit selectionChanged();
}

/*!
	Finds the drawable instance of the given abstract music element.

	\sa findCElement()
*/
CADrawableMusElement *CAScoreView::findMElement(CAMusElement *elt) {
	for (int i=0; i<_drawableMList.size(); i++)
		if ( static_cast<CADrawableMusElement*>(_drawableMList.at(i))->musElement()==elt )
			return static_cast<CADrawableMusElement*>(_drawableMList.at(i));
	return 0;
}

/*!
	Finds the drawable instance of the given abstract context.

	\sa findMElement()
*/
CADrawableContext *CAScoreView::findCElement(CAContext *context) {
	for (int i=0; i<_drawableCList.size(); i++)
		if (static_cast<CADrawableContext*>(_drawableCList.at(i))->context()==context)
			return static_cast<CADrawableContext*>(_drawableCList.at(i));
	return 0;
}

/*!
	Creates a CATextEdit widget over the existing drawable syllable \a dMusElt.
	Returns the pointer to the created widget.

	\sa createTextEdit( QRect geometry )
*/
CATextEdit *CAScoreView::createTextEdit( CADrawableMusElement *dMusElt ) {
	if ( !dMusElt || !dMusElt->musElement() )
		return 0;

	int xPos=dMusElt->pos().x(), yPos=dMusElt->pos().y(),
	    width=100, height=25;
	QString text;
	if ( dMusElt->musElement()->musElementType()==CAMusElement::Syllable ) {
		CADrawableLyricsContext *dlc = static_cast<CADrawableLyricsContext*>(dMusElt->drawableContext());
		CASyllable *syllable = static_cast<CASyllable*>(dMusElt->musElement());
		if (!dlc || !syllable) return 0;

		CADrawableMusElement *dRight = findMElement( dlc->lyricsContext()->next( syllable ) );
		if (dRight)
			width = dRight->pos().x() - dMusElt->pos().x();

		text = syllable->text();
		if (syllable->hyphenStart()) text+="-";
		else if (syllable->melismaStart()) text+="_";
	} else if ( dMusElt->musElement()->musElementType()==CAMusElement::Mark ) {
		CAMusElement *elt = dMusElt->musElement();
		if ( static_cast<CAMark*>(elt)->markType()==CAMark::Text ) {
			text = static_cast<CAText*>(elt)->text();
		} else if ( static_cast<CAMark*>(elt)->markType()==CAMark::BookMark ) {
			text = static_cast<CABookMark*>(elt)->text();
		}
	}

	textEdit()->setText(text);
	setTextEditVisible( true );
	setTextEditGeometry( QRect(xPos-2, yPos, width+2, height) );
	updateHelpers(); // show it
	textEdit()->setFocus();

	return textEdit();
}

/*!
	Removes and deletes the text edit when quitting text editing mode.
*/
void CAScoreView::removeTextEdit() {
	setTextEditVisible( false ); // don't delete it, just hide it!
	updateHelpers();
	textEdit()->setText("");
	this->setFocus();
}

/*!
	Returns the maximum X of the viewable World a little bigger to make insertion at the end easy.
*/
template <typename T>
int CAScoreView::getMaxXExtended(CAKDTree<T> &v) {
	return v.getMaxX() + RIGHT_EXTRA_SPACE;
}

/*!
	Returns the maximum Y of the viewable World a little bigger to make insertion at the end easy.
*/
template <typename T>
int CAScoreView::getMaxYExtended(CAKDTree<T> &v) {
	return v.getMaxY() + BOTTOM_EXTRA_SPACE;
}

/*!
	Returns a list of drawable contexts the current score view includes between
	the vertical coordinates \a y1 and \a y2.
	The context is in a list already if only part of the context is touched by the region.
	That is the first returned context's top border is smaller than \a y1 and the last returned context's
	bottom border is larger than \a x2.
*/
QList<CADrawableContext*> CAScoreView::findContextsInRegion( QRect &region ) {
	return _drawableCList.findInRange(region);
}

/*!
	Returns Canorus time for the given X coordinate \a x.

	Returns 0, if no contexts are present.
*/
int CAScoreView::coordsToTime( double x ) {
	CADrawableMusElement *d1 = nearestLeftElement( x, 0 );
	if ( selection().contains(d1) ) {
		_drawableMList.list().removeAll(d1);
		CADrawableMusElement *newD1 = nearestLeftElement( x, 0 );
		_drawableMList.addElement(d1);
		d1 = newD1;
	}

	CADrawableMusElement *d2 = nearestRightElement( x, 0 );
	if ( selection().contains(d2) ) {
		_drawableMList.list().removeAll(d2);
		CADrawableMusElement *newD2 = nearestRightElement( x, 0 );
		_drawableMList.addElement(d2);
		d2 = newD2;
	}

	if ( d1 && d2 && d1->musElement() && d2->musElement() ) {
		int delta = (d2->pos().x() - d1->pos().x());
		if (!delta) delta=1;
		return qRound(d1->musElement()->timeStart() + ( d2->musElement()->timeStart() - d1->musElement()->timeStart() ) * ( (x - d1->pos().x()) / (float)delta ) );
	} else if ( d1 && d1->musElement() )
		return ( d1->musElement()->timeEnd() );
	else
		return 0;
}

/*!
	Simple Version of \sa timeToCoords( time ):
	Returns the X coordinate for the given Canorus \a time.
	Returns -1, if such a time doesn't exist in the score.
*/
double CAScoreView::timeToCoordsSimpleVersion( int time ) {
	CADrawableMusElement *leftElt = 0;
	CADrawableMusElement *rightElt = 0;
	for (int i=0; i<_drawableMList.size(); i++) {
		//if ( selection().contains( _drawableMList.at(i) ) ) {
		//	std::cout << "  in continue-Ast" << std::endl;
		//	continue;
		//}
		if ( _drawableMList.at(i)->musElement() && _drawableMList.at(i)->musElement()->timeStart() <= time && (
				!leftElt || _drawableMList.at(i)->musElement()->timeStart() > leftElt->musElement()->timeStart() ||
		         _drawableMList.at(i)->pos().x() > leftElt->pos().x() ) // get the right-most element of that time
		   )
			leftElt = _drawableMList.at(i);

		if ( _drawableMList.at(i)->musElement() && _drawableMList.at(i)->musElement()->timeStart() >= time && (
				!rightElt || _drawableMList.at(i)->musElement()->timeStart() < rightElt->musElement()->timeStart() ||
		        _drawableMList.at(i)->pos().x() < rightElt->pos().x() ) // get the left-most element of that time
		   )
			rightElt = _drawableMList.at(i);
	}

	if ( leftElt /* && rightElt */ && leftElt->musElement() /* && rightElt->musElement() */ ) {
/*
		int delta = (rightElt->musElement()->timeStart() - leftElt->musElement()->timeStart());
		if (!delta) delta=1;
		return qRound(leftElt->pos().x() + ( rightElt->pos().x() - leftElt->pos().x() ) *
		              ( ((float)time - leftElt->musElement()->timeStart()) / delta )
		             );
*/
		return leftElt->pos().x();
	} else {
		return -1;
	}
}

/*!
	Returns the X coordinate for the given Canorus \a time.
	Returns -1, if such a time doesn't exist in the score.
*/
double CAScoreView::timeToCoords( int time ) {
	CADrawableMusElement *leftElt = 0;
	CADrawableMusElement *rightElt = 0;
	for (int i=0; i<_drawableMList.size(); i++) {
		if ( selection().contains( _drawableMList.at(i) ) )
			continue;
		if ( _drawableMList.at(i)->musElement() && _drawableMList.at(i)->musElement()->timeStart() <= time && (
				!leftElt || _drawableMList.at(i)->musElement()->timeStart() > leftElt->musElement()->timeStart() ||
		         _drawableMList.at(i)->pos().x() > leftElt->pos().x() ) // get the right-most element of that time
		   )
			leftElt = _drawableMList.at(i);

		if ( _drawableMList.at(i)->musElement() && _drawableMList.at(i)->musElement()->timeStart() >= time && (
				!rightElt || _drawableMList.at(i)->musElement()->timeStart() < rightElt->musElement()->timeStart() ||
		        _drawableMList.at(i)->pos().x() < rightElt->pos().x() ) // get the left-most element of that time
		   )
			rightElt = _drawableMList.at(i);
	}

	if ( leftElt && rightElt && leftElt->musElement() && rightElt->musElement() ) {
		int delta = (rightElt->musElement()->timeStart() - leftElt->musElement()->timeStart());
		if (!delta) delta=1;
		return leftElt->pos().x() + ( rightElt->pos().x() - leftElt->pos().x() ) *
		              ( ((double)time - leftElt->musElement()->timeStart()) / delta );
	} else {
		return -1;
	}
}

void CAScoreView::setShadowNoteLength( CAPlayableLength l ) {
	for (int i=0; i<_shadowNote.size(); i++) {
		_shadowNote[i]->setPlayableLength( l );
	}

	updateHelpers();
}

/*!
	Returns a list of currently selected music elements.
	Does the same as selection(), but doesn't return their Drawable instances.
 */
QList<CAMusElement*> CAScoreView::musElementSelection() {
	QList<CAMusElement*> res;

	for (int i=0; i<_selection.size(); i++) {
		if (!res.contains(_selection[i]->musElement())) {
			res << _selection[i]->musElement();
		}
	}

	return res;
}

/*!
	\fn CASheet *CAScoreView::sheet()
	Returns the pointer to the view's sheet it represents.
*/

/*!
	\fn bool CAScoreView::removeFromSelection(CADrawableMusElement *elt)
	Removes the given drawable music element \a elt from the selection, if it exists.
	Returns True, if element existed in the selection and was removed, false otherwise.
*/

/*!
	\fn void CAScoreView::clearSelection()
	Clears the current selection. Its behaviour is the same as calling clearMSelection() and clearCSelection().
*/

/*!
	\fn QList<CAMusElement*> CAScoreView::selection()
	Returns a list of the currently selected drawable music elements.
*/

/*!
	\var bool CAScoreView::_allowManualScroll
	This property holds whether a user interaction with the scrollbars actually triggers the scroll of the view.
*/

/*!
	\fn float CAScoreView::zoom()
	Returns the zoom level of the view (1.0 = 100%, 1.5 = 150% etc.).
*/

/*!
	\fn void CAScoreView::CAMousePressEvent(QMouseEvent *e, QPoint p, CAScoreView *v)

	This signal is emitted when mousePressEvent() is called. Parent class is usually connected to this event.
	It adds another two arguments to the mousePressEvent() function - pointer to this view and coordinates
	in world coordinates where user used the mouse.
	This is useful when a parent class wants to know which class the signal was emmitted by.

	\param e Mouse event which gets processed.
	\param p Coordinates of the mouse cursor in absolute world values.
	\param v Pointer to this view (the view which emmitted the signal).
*/

/*!
	\fn void CAScoreView::CAMouseMoveEvent(QMouseEvent *e, QPoint p, CAScoreView *v)

	This signal is emitted when mouseMoveEvent() is called. Parent class is usually connected to this event.
	It adds another two arguments to the mouseMoveEvent() function - pointer to this view and coordinates
	in world coordinates where user used the mouse.
	This is useful when a parent class wants to know which class the signal was emmitted by.

	\param e Mouse event which gets processed.
	\param p Coordinates of the mouse cursor in absolute world values.
	\param v Pointer to this view (the view which emmitted the signal).
*/

/*!
	\fn void CAScoreView::CAMouseReleaseEvent(QMouseEvent *e, QPoint p, CAScoreView *v)

	This signal is emitted when mouseReleaseEvent() is called. Parent class is usually connected to this event.
	It adds another two arguments to the mouseReleaseEvent() function - pointer to this score view and coordinates
	in world coordinates where user used the mouse.
	This is useful when a parent class wants to know which class the signal was emmitted by.

	\param e Mouse event which gets processed.
	\param p Coordinates of the mouse cursor in absolute world values.
	\param v Pointer to this view (the view which emmitted the signal).
*/

/*!
	\fn void CAScoreView::CAWheelEvent(QWheelEvent *e, QPoint p, CAScoreView *v)

	This signal is emitted when wheelEvent() is called. Parent class is usually connected to this event.
	It adds another two arguments to the wheelEvent() function - pointer to this score view and coordinates
	in world coordinates where user used the mouse.
	This is useful when a parent class wants to know which class the signal was emmitted by.

	\param e Wheel event which gets processed.
	\param p Coordinates of the mouse cursor in absolute world values.
	\param v Pointer to this view (the view which emmitted the signal).
*/

/*!
	\fn void CAScoreView::CAKeyPressEvent(QKeyEvent *e, CAScoreView *v)

	This signal is emitted when keyPressEvent() is called. Parent class is usually connected to this event.
	It adds another two arguments to the wheelEvent() function - pointer to this score view.
	This is useful when a parent class wants to know which class the signal was emmitted by.

	\param e Wheel event which gets processed.
	\param v Pointer to this view (the view which emmitted the signal).
*/
