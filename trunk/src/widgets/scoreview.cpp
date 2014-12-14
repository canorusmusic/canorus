/*!
	Copyright (c) 2006-2009, Matevž Jekovec, Canorus development team
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
	_worldX = _worldY = 0;
	_worldW = _worldH = 0;
	_zoom = 1.0;
	_holdRepaint = false;
	_hScrollBarDeadLock = false;
	_vScrollBarDeadLock = false;
	_checkScrollBarsDeadLock = false;
	_playing = false;
	_currentContext = 0;
	_xCursor = _yCursor = 0;
	setResizeDirection( CADrawable::Undefined );

	// init layout
	_layout = new QGridLayout(this);
	_layout->setMargin(2);
	_layout->setSpacing(2);
	_drawBorder = false;
	_grabTabKey = true;
	setFocusPolicy( Qt::StrongFocus );

	// init virtual canvas
	_canvas = new QWidget(this);
	setMouseTracking(true);
	_canvas->setMouseTracking(true);
	_repaintArea = 0;

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

	_hScrollBar->disconnect();
	_vScrollBar->disconnect();
}

void CAScoreView::on_animationTimer_timeout() {
	_animationStep++;

	float newZoom = _zoom + (_targetZoom - _zoom) * sqrt(((double)_animationStep)/ANIMATION_STEPS);
	double newWorldX = _worldX + (_targetWorldX - _worldX) * sqrt(((double)_animationStep)/ANIMATION_STEPS);
	double newWorldY = _worldY + (_targetWorldY - _worldY) * sqrt(((double)_animationStep)/ANIMATION_STEPS);
	double newWorldW = drawableWidth() / newZoom;
	double newWorldH = drawableHeight() / newZoom;

	setWorldCoords(newWorldX, newWorldY, newWorldW, newWorldH);

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
	_mapDrawable.insertMulti(elt->musElement(), elt);
	if (select) {
		_selection.clear();
		addToSelection(elt);
	}

	elt->drawableContext()->addMElement(elt);
	emit selectionChanged();
}

/*!
	Adds a drawable music element \a elt to the score view and selects it, if \a select is true.
*/
void CAScoreView::addCElement(CADrawableContext *elt, bool select) {
	_drawableCList.addElement(elt);
	_mapDrawable.insertMulti(elt->context(), elt);

	if (select)
		setCurrentContext(elt);

	if (elt->drawableContextType() == CADrawableContext::DrawableStaff &&
	    static_cast<CAStaff*>(elt->context())->voiceList().size()) {
		_shadowNote << new CANote( CADiatonicPitch(), CAPlayableLength(CAPlayableLength::Quarter, 0), static_cast<CAStaff*>(elt->context())->voiceList()[0], 0 );
		_shadowDrawableNote << new CADrawableNote(_shadowNote.back(), elt, 0, 0, true);
	}
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
	
	QList<CADrawableContext*> drawableContexts = _drawableCList.list();
	for (int i=0; i<drawableContexts.size(); i++) {
		CAContext *c = drawableContexts[i]->context();
		if (c == context) {
			setCurrentContext(drawableContexts[i]);
			return drawableContexts[i];
		}
	}

	return 0;
}

/*!
	Change the x-coord of the last mouse press coords to after the right most element on the given list.
*/
void CAScoreView::setLastMousePressCoordsAfter(const QList<CAMusElement*> list) {
	double maxX = 0;
	for( int i=0; i < list.size(); i++ ) {
		QList<CADrawable*> drawables = _mapDrawable.values(list[i]);
		for ( int j=0; j<drawables.size(); j++) {
			maxX = qMax(drawables[j]->xPos() + drawables[j]->width(), maxX);
		}
	}
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
	for (int i=0; i<l.size(); i++)
		if ( !l[i]->isSelectable() || (selectedVoice() && l[i]->musElement() && l[i]->musElement()->isPlayable() && static_cast<CAPlayable*>(l[i]->musElement())->voice()!=selectedVoice()) )
			l.removeAt(i--);

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
	
	QList<CADrawable*> drawables = _mapDrawable.values(elt);
	for (int i=0; i<drawables.size(); i++) {
		if ( drawables[i]->drawableType()==CADrawable::DrawableMusElement &&
		     static_cast<CADrawableMusElement*>(drawables[i])->musElement() == elt &&
		     drawables[i]->isSelectable() ) {
			addToSelection(static_cast<CADrawableMusElement*>(drawables[i]));
		}
	}

	emit selectionChanged();

	if ( selection().size() )
		return selection().back();
	else
		return 0;
}

void CAScoreView::importElements(CAKDTree<CADrawableMusElement*> *origDMusElts, CAKDTree<CADrawableContext*> *origDContexts)
{
	QList<CADrawableContext*> drawableContexts = origDContexts->list();
	for (int i=0; i<drawableContexts.size(); i++) {
		addCElement(drawableContexts[i]->clone());
	}
	QList<CADrawableContext*> newDrawableContexts = _drawableCList.list();
	
	QList<CADrawableMusElement*> drawableMusElements = origDMusElts->list();
	for (int i=0; i<drawableMusElements.size(); i++)
	{
		int idx = drawableContexts.indexOf(drawableMusElements[i]->drawableContext());
		if(idx == -1)
		{
			std::cerr << "ERROR: Music element " << drawableMusElements[i] << " does not have a drawable context!" << std::endl;
		} else
		if (newDrawableContexts.size()<=idx) {
			std::cerr << "ERROR: Index out of range. " << newDrawableContexts.size() << " new drawable contexts created. Request for idx=" << idx << std::endl;
		} else {
			addMElement( drawableMusElements[i]->clone(newDrawableContexts[idx]) );
		}
	}
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
	_mapDrawable.clear();

	CALayoutEngine::reposit(this);

	for (int i=0; i<_shadowNote.size(); i++) {
		_shadowNote[i]->setPlayableLength(l);
	}

	if (contextIdx != -1)	// restore the last used context
		setCurrentContext((CADrawableContext*)((_drawableCList.size() > contextIdx)?_drawableCList.list().at(contextIdx):0));
	else
		setCurrentContext(0);

	addToSelection(musElementSelection);

	setWorldCoords( worldCoords() ); // needed to update the scrollbars
	checkScrollBars();
	updateHelpers();
}

/*!
	Sets the world Top-Left X coordinate of the view. Animates the scroll, if \a animate is True.
	If \a force is True, sets the value despite the potential illegal value (like negative coordinates).

	\warning Repaint is not done automatically!
*/
void CAScoreView::setWorldX(int x, bool animate, bool force) {
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
	Sets the world Top-Left Y coordinate of the view. Animates the scroll, if \a animate is True.
	If \a force is True, sets the value despite the potential illegal value (like negative coordinates).

	\warning Repaint is not done automatically!
*/
void CAScoreView::setWorldY(int y, bool animate, bool force) {
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
	Sets the world width of the view.
	If \a force is True, sets the value despite the potential illegal value (like negative coordinates).

	\warning Repaint is not done automatically!
*/
void CAScoreView::setWorldWidth(int w, bool force) {
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
	Sets the world height of the view.
	If \a force is True, sets the value despite the potential illegal value (like negative coordinates).

	\warning Repaint is not done automatically!
*/
void CAScoreView::setWorldHeight(int h, bool force) {
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
	Sets the world coordinates of the view to the given rectangle \a coords.
	This is an overloaded member function, provided for convenience.

	\warning Repaint is not done automatically!
*/
void CAScoreView::setWorldCoords(QRect coords, bool animate, bool force) {
	_checkScrollBarsDeadLock = true;

	if (!drawableWidth() && !drawableHeight())
		return;

	float scale = (float)drawableWidth() / drawableHeight();	//always keep the world rectangle area in the same scale as the actual width/height of the drawable canvas
	if (coords.height()) {	//avoid division by zero
		if (coords.width() / coords.height() > scale)
			coords.setHeight( coords.width() / scale );
		else
			coords.setWidth( coords.height() * scale );
	} else
		coords.setHeight( coords.width() / scale );


	setWorldWidth(coords.width(), force);
	setWorldHeight(coords.height(), force);
	setWorldX(coords.x(), animate, force);
	setWorldY(coords.y(), animate, force);
	_checkScrollBarsDeadLock = false;

	checkScrollBars();
}

/*!
	\fn void CAScoreView::setWorldCoords(int x, int y, int w, int h, bool animate, bool force)
	Sets the world coordinates of the view.
	This is an overloaded member function, provided for convenience.

	\warning Repaint is not done automatically!

	\param x Top-left X coordinate of the new view area in absolute world units.
	\param y Top-left Y coordinate of the new view area in absolute world units.
	\param w Width of the new view area in absolute world units.
	\param h Height of the new view area in absolute world units.
	\param animate Use animated scroll.
	\param force Use the given world units despite their illegal values (like negative coordinates etc.).
*/

void CAScoreView::zoomToSelection(bool animate, bool force) {
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

void CAScoreView::zoomToWidth(bool animate, bool force) {
	int maxX = (getMaxXExtended(_drawableCList)>getMaxXExtended(_drawableMList))?getMaxXExtended(_drawableCList):getMaxXExtended(_drawableMList);
	setWorldCoords(0,0,maxX,0,animate,force);
}

void CAScoreView::zoomToHeight(bool animate, bool force) {
	int maxY = (getMaxYExtended(_drawableCList)>getMaxYExtended(_drawableMList))?getMaxYExtended(_drawableCList):getMaxYExtended(_drawableMList);
	setWorldCoords(0,0,0,maxY,animate,force);
}

void CAScoreView::zoomToFit(bool animate, bool force) {
	int maxX = ((_drawableCList.getMaxX() > _drawableMList.getMaxX())?_drawableCList.getMaxX():_drawableMList.getMaxX());
	int maxY = ((_drawableCList.getMaxY() > _drawableMList.getMaxY())?_drawableCList.getMaxY():_drawableMList.getMaxY());

	setWorldCoords(0, 0, maxX, maxY, animate, force);
}

/*!
	Sets the world coordinates of the view, so the given coordinates are the center of the new view area.
	If the area has for eg. negative top-left coordinates, the area is moved to the (0,0) coordinates if \a force is False.
	View's width and height stay intact.
	\warning Repaint is not done automatically!
*/
void CAScoreView::setCenterCoords(double x, double y, bool animate, bool force) {
	_checkScrollBarsDeadLock = true;
	setWorldX(x - 0.5*_worldW, animate, force);
	setWorldY(y - 0.5*_worldH, animate, force);
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
void CAScoreView::setZoom(float z, double x, double y, bool animate, bool force) {
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
			_targetWorldX = 1.5*_worldX + 0.25*_worldW - 0.5*x;
			_targetWorldY = 1.5*_worldY + 0.25*_worldH - 0.5*y;
			_targetZoom = z;
			startAnimationTimer();
			return;
		}
	}

	//set the world width - updates the zoom level zoom_ as well
	setWorldWidth(drawableWidth() / z);
	setWorldHeight(drawableHeight() / z);

	if (!zoomOut) { //zoom in
		//the new view's center coordinates will become the middle point of the current view center coords and the mouse pointer coords
		setCenterCoords( ( _worldX + (_worldW/2) + x ) / 2,
		                 ( _worldY + (_worldH/2) + y ) / 2,
		                 force );
	} else { //zoom out
		//the new view's center coordinates will become the middle point of the current view center coords and the mirrored over center pointer coords
		//worldX_ + (worldW_/2) + (worldX_ + (worldW_/2) - x)/2
		setCenterCoords( 1.5*_worldX + 0.75*_worldW - 0.5*x,
		                 1.5*_worldY + 0.75*_worldH - 0.5*y,
		                 force );
	}

	checkScrollBars();
	updateHelpers();
}

/*!
	\fn void CAScoreView::setZoom(float z, QPoint p, bool animate, bool force);
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
#include <time.h> //benchmarking
#include <sys/time.h> //benchmarking
void CAScoreView::paintEvent(QPaintEvent *e) {
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
	timeval timeStart, timeEnd, timeEnd2;
	gettimeofday(&timeStart, NULL);
	QList<CADrawableContext*> cList;
	//int j = _drawableCList.size();
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
		               ((_currentContext == cList[i])?selectedContextColor():foregroundColor()),
		               _worldX,
		               _worldY
		};
		cList[i]->draw(&p, s);
	}

	// draw music elements
	QList<CADrawableMusElement*> mList;
	if (_repaintArea)
		mList = _drawableMList.findInRange(_repaintArea->x(), _repaintArea->y(), _repaintArea->width(),_repaintArea->height());
	else
		mList = _drawableMList.findInRange(_worldX, _worldY, _worldW, _worldH);

	gettimeofday(&timeEnd, NULL);

	p.setRenderHint( QPainter::Antialiasing, CACanorus::settings()->antiAliasing() );

	for (int i=0; i<mList.size(); i++) {
		QColor color;
		CAMusElement *elt = mList[i]->musElement();
		
		// determine element color (based on selection, current mode, active voice etc.)
		if ( _selection.contains(mList[i])) {
			color = selectionColor();
		} else
		if ( (selectedVoice() &&
		     ((elt &&
		      ((elt->isPlayable() && static_cast<CAPlayable*>(elt)->voice()==selectedVoice()) ||
		       (!elt->isPlayable() && elt->context()==selectedVoice()->staff()) ||
		       elt->context()!=selectedVoice()->staff())) ||
		      (!elt && mList[i]->drawableContext()->context()==selectedVoice()->staff())
		     )) ||
		     (!selectedVoice())
		   ) {
			if ( elt && elt->musElementType()==CAMusElement::Rest &&
			     static_cast<CAPlayable*>(elt)->voice()==selectedVoice() &&
			     static_cast<CARest*>(elt)->restType()==CARest::Hidden
			   ) {
			   	color = hiddenElementsColor();
			} else if ( (elt && elt->musElementType()==CAMusElement::Rest &&
			            static_cast<CARest*>(elt)->restType()==CARest::Hidden) ||
			            (elt && !elt->isVisible())
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
		               _zoom,
		               qRound((mList[i]->xPos() - _worldX) * _zoom),
		               qRound((mList[i]->yPos() - _worldY) * _zoom),
		               drawableWidth(), drawableHeight(),
		               color,
		               _worldX,
		               _worldY
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
			_zoom,
			qRound( (selectionRegionList().at(i).x() - _worldX) * _zoom),
			qRound( (selectionRegionList().at(i).y() - _worldY) * _zoom),
			qRound( selectionRegionList().at(i).width() * _zoom),
			qRound( selectionRegionList().at(i).height() * _zoom),
			selectionAreaColor(),
            _worldX,
            _worldY
		};
		drawSelectionRegion( &p, c );
	}

	// draw shadow note
	if (_shadowNoteVisible) {
		for (int i=0; i<_shadowDrawableNote.size(); i++) {
			if ( CACanorus::settings()->shadowNotesInOtherStaffs() || _shadowDrawableNote[i]->drawableContext() == currentContext() ) {
				CADrawSettings s = {
					_zoom,
					qRound((_shadowDrawableNote[i]->xPos() - _worldX - _shadowDrawableNote[i]->width()/2) * _zoom),
					qRound((_shadowDrawableNote[i]->yPos() - _worldY) * _zoom),
					drawableWidth(), drawableHeight(),
					disabledElementsColor(),
	               _worldX,
	               _worldY
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

		// draw note name
		if (_shadowNote.size()) {
			QFont font("FreeSans");
			font.setPixelSize( 20 );
			p.setFont(font);
			p.setPen(disabledElementsColor());
			p.drawText( qRound((_xCursor-_worldX+10) * _zoom), qRound((_yCursor-_worldY-10) * _zoom), CANote::generateNoteName(_shadowNote[0]->diatonicPitch().noteName(), _shadowNoteAccs) );
		}
	}
	gettimeofday(&timeEnd2, NULL);
	
//	std::cout << "finding elements to draw took " << timeEnd.tv_sec-timeStart.tv_sec+(timeEnd.tv_usec-timeStart.tv_usec)/1000000.0 << "s." << std::endl;
//	std::cout << "painting " << mList.size() << " elements took " << timeEnd2.tv_sec-timeEnd.tv_sec+(timeEnd2.tv_usec-timeEnd.tv_usec)/1000000.0 << "s." << std::endl;

	// flush the oldWorld coordinates as they're needed for the first repaint only
	_oldWorldX = _worldX; _oldWorldY = _worldY;
	_oldWorldW = _worldW; _oldWorldH = _worldH;

	if (_repaintArea) {
		delete _repaintArea;
		_repaintArea = 0;
		p.setClipping(false);
	}
}

void CAScoreView::updateHelpers() {
	// Shadow notes
	if (currentContext()?(currentContext()->drawableContextType() == CADrawableContext::DrawableStaff):0) {
		int pitch = (static_cast<CADrawableStaff*>(currentContext()))->calculatePitch(_xCursor, _yCursor);	// the current staff has the real pitch we need
		for (int i=0; i<_shadowNote.size(); i++) {	// apply this pitch to all shadow notes in all staffs
			//CAClef *clef = (static_cast<CADrawableStaff*>(_shadowDrawableNote[i]->drawableContext()))->getClef( _xCursor );
			CADiatonicPitch dPitch(pitch, 0);
			_shadowNote[i]->setDiatonicPitch( dPitch );

			if (selectedVoice()) {
				_shadowNote[i]->setStemDirection( selectedVoice()->stemDirection() );
			}

			CADrawableContext *c = _shadowDrawableNote[i]->drawableContext();
			delete _shadowDrawableNote[i];
			_shadowDrawableNote[i] = new CADrawableNote(_shadowNote[i], c, _xCursor, static_cast<CADrawableStaff*>(c)->calculateCenterYCoord(pitch, _xCursor), true);
		}
	}

	// Text edit widget
	if ( textEditVisible() ) {
		textEdit()->setFont( QFont("Century Schoolbook L", qRound(zoom()*(12-2))) );
		textEdit()->setGeometry(
			qRound( (textEditGeometry().x()-worldX())*zoom() ),
			qRound( (textEditGeometry().y()-worldY())*zoom() ),
			qRound( textEditGeometry().width()*zoom() ),
			qRound( textEditGeometry().height()*zoom() )
		);
		textEdit()->show();
	} else {
		textEdit()->hide();
	}
}

void CAScoreView::drawSelectionRegion( QPainter *p, CADrawSettings s ) {
	p->fillRect(s.x, s.y, s.w, s.h, QBrush(s.color));
}

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
	Note that repaint() event is also triggered when the internal drawable canvas changes its size (for eg. when scrollbars are shown/hidden) and the size of the view does not change.
*/
void CAScoreView::resizeEvent(QResizeEvent *e) {
	setWorldCoords( _worldX, _worldY, drawableWidth() / _zoom, drawableHeight() / _zoom );
	// setWorld methods already check for scrollbars
}

/*!
	Checks whether the scrollbars are needed (the whole scene is not rendered) or not.
	Scrollbars get shown or hidden here.
	Repaint is done automatically, if needed.
*/
void CAScoreView::checkScrollBars() {
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
	return QWidget::event(event);
}

/*!
	Processes the mousePressEvent().
	A new signal is emitted: CAMousePressEvent(), which usually gets processed by the parent class then.
*/
void CAScoreView::mousePressEvent(QMouseEvent *e) {
	CAView::mousePressEvent(e);
	QPoint coords(e->x() / _zoom + _worldX, e->y() / _zoom + _worldY);
	if ( selection().size() && selection()[0]->isHScalable() && coords.y()>=selection()[0]->yPos() && coords.y()<=selection()[0]->yPos()+selection()[0]->height() ) {
		if ( coords.x()==selection()[0]->xPos()  ) {
			setResizeDirection(CADrawable::Left);
		} else if ( coords.x()==selection()[0]->xPos()+selection()[0]->width() ) {
			setResizeDirection(CADrawable::Right);
		}
	} else if ( selection().size() && selection().at(0)->isVScalable() && coords.x()>=selection()[0]->xPos() && coords.x()<=selection()[0]->xPos()+selection()[0]->width() ) {
		if ( coords.y()==selection()[0]->yPos() ) {
			setResizeDirection(CADrawable::Top);
		} else if ( coords.y()==selection()[0]->yPos()+selection()[0]->height() ) {
			setResizeDirection(CADrawable::Bottom);
		}
	}

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
	emit CAMouseReleaseEvent(e, QPoint(e->x() / _zoom + _worldX, e->y() / _zoom + _worldY));
	setResizeDirection( CADrawable::Undefined );
}

/*!
	Processes the mouseMoveEvent().
	A new signal is emitted: CAMouseMoveEvent(), which usually gets processed by the parent class then.
*/
void CAScoreView::mouseMoveEvent(QMouseEvent *e) {
	QPoint coords(e->x() / _zoom + _worldX, e->y() / _zoom + _worldY);

	_xCursor = coords.x();
	_yCursor = coords.y();

	bool isHScalable = false;
	for ( int i=0; i<selection().size() && !isHScalable; i++) {
		if ( selection()[i]->isHScalable() && (coords.x()==selection()[i]->xPos() || coords.x()==selection()[i]->xPos()+selection()[i]->width()) &&
		     coords.y()>=selection()[i]->yPos() && coords.y()<=selection()[i]->yPos()+selection()[i]->height()
		   )
			isHScalable = true;
	}

	bool isVScalable = false;
	for ( int i=0; i<selection().size() && !isVScalable; i++) {
		if ( selection()[i]->isVScalable() && (coords.y()==selection()[i]->yPos() || coords.y()==selection()[i]->yPos()+selection()[i]->height()) &&
		     coords.x()>=selection()[i]->xPos() && coords.x()<=selection()[i]->xPos()+selection()[i]->width()
		   )
			isVScalable = true;
	}

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
void CAScoreView::wheelEvent(QWheelEvent *e) {
	QPoint coords((int)(e->x() / _zoom) + _worldX, (int)(e->y() / _zoom) + _worldY);

	emit CAWheelEvent(e, coords);

	_xCursor = (int)(e->x() / _zoom) + _worldX;	//TODO: _xCursor and _yCursor are still the old one. Somehow, _zoom level and _worldX/Y are not updated when emmiting CAWheel event. -Matevz
	_yCursor = (int)(e->y() / _zoom) + _worldY;
}

/*!
	Processes the keyPressEvent().
	A new signal is emitted: CAKeyPressEvent(), which usually gets processed by the parent class then.
*/
void CAScoreView::keyPressEvent(QKeyEvent *e) {
	emit CAKeyPressEvent(e);
}

void CAScoreView::setScrollBarVisible(CAScrollBarVisibility status) {
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
void CAScoreView::HScrollBarEvent(int val) {
	if ((_allowManualScroll) && (!_hScrollBarDeadLock)) {
		setWorldX(val);
		repaint();
	}
}

/*!
	Processes the Vertical scroll bar event.
	This method is called when the horizontal scrollbar changes its value, let it be internally or due to user interaction.
*/
void CAScoreView::VScrollBarEvent(int val) {
	if ((_allowManualScroll) && (!_vScrollBarDeadLock)) {
		setWorldY(val);
		repaint();
	}
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
	for (i=0; i<_selection.size() && _selection[i]->xPos() < elt->xPos(); i++);

	if ( elt->isSelectable() )
		_selection.insert( i, elt );

	if ( triggerSignal )
		emit selectionChanged();
}

/*!
	Adds the given list of drawable music elements \a list to the current selection.
*/
void CAScoreView::addToSelection(const QList<CADrawableMusElement*> list, bool selectableOnly ) {
	for (int i=0; i<list.size(); i++) {
		if ( !selectableOnly || (selectableOnly && list[i]->isSelectable()) )
			addToSelection(list[i], false);
	}

	emit selectionChanged();
}

/*!
	Adds the drawable music element of the given abstract music element \a elt to the selection.
	Returns a pointer to its drawable element or 0, if the music element is not part of this score view.
*/
CADrawableMusElement *CAScoreView::addToSelection(CAMusElement *elt) {
	QList<CADrawable*> l = _mapDrawable.values(elt);
	for (int i=0; i<l.size(); i++) {
		addToSelection(static_cast<CADrawableMusElement*>(l[i]));
	}

	emit selectionChanged();
	return _selection.back();
}

/*!
	Adds the given list of abstract music elements to the selection.
*/
void CAScoreView::addToSelection(const QList<CAMusElement*> elts) {
	for (int i=0; i<elts.size(); i++) {
		QList<CADrawable*> l = _mapDrawable.values(elts[i]);
		for (int j=0; j<l.size(); j++) {
			addToSelection(static_cast<CADrawableMusElement*>(l[j]), false);
		}
	}

	emit selectionChanged();
}

/*!
	Select all elements in the view.
	This function is usually associated with CTRL+A key.
*/
void CAScoreView::selectAll() {
	clearSelection();

	QList<CADrawableMusElement*> elts = _drawableMList.list();
	for(int i=0; i<elts.size(); i++)
		addToSelection(elts[i], false);

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

	QList<CADrawableMusElement*> elts = _drawableMList.list();
	for(int i=0; i<elts.size(); i++)
		if(!oldSelection.contains(elts[i]))
			addToSelection(elts[i], false);

	emit selectionChanged();
}

/*!
	Finds the first drawable instance of the given abstract music element.

	\sa findCElement()
*/
CADrawableMusElement *CAScoreView::findMElement(CAMusElement *elt) {
	QList<CADrawable*> hits = _mapDrawable.values(elt);
	if (hits.size()) {
		return static_cast<CADrawableMusElement*>(hits[0]);
	}
	return 0;
}

/*!
	Finds the drawable instance of the given abstract context.

	\sa findMElement()
*/
CADrawableContext *CAScoreView::findCElement(CAContext *context) {
	QList<CADrawable*> hits = _mapDrawable.values(context);
	if (hits.size()) {
		return static_cast<CADrawableContext*>(hits[0]);
	}
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

	int xPos=dMusElt->xPos(), yPos=dMusElt->yPos(),
	    width=100, height=25;
	QString text;
	if ( dMusElt->musElement()->musElementType()==CAMusElement::Syllable ) {
		CADrawableLyricsContext *dlc = static_cast<CADrawableLyricsContext*>(dMusElt->drawableContext());
		CASyllable *syllable = static_cast<CASyllable*>(dMusElt->musElement());
		if (!dlc || !syllable) return 0;

		CADrawableMusElement *dRight = findMElement( dlc->lyricsContext()->next( syllable ) );
		if (dRight)
			width = dRight->xPos() - dMusElt->xPos();

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
		CADrawableMusElement *newD1 = nearestLeftElement( d1->xPos(), 0 );
		d1 = newD1;
	}

	CADrawableMusElement *d2 = nearestRightElement( x, 0 );
	if ( selection().contains(d2) ) {
		CADrawableMusElement *newD2 = nearestRightElement( d2->xPos()+d2->width(), 0 );
		d2 = newD2;
	}

	if ( d1 && d2 && d1->musElement() && d2->musElement() ) {
		int delta = (d2->xPos() - d1->xPos());
		if (!delta) delta=1;
		return qRound(d1->musElement()->timeStart() + ( d2->musElement()->timeStart() - d1->musElement()->timeStart() ) * ( (x - d1->xPos()) / (float)delta ) );
	} else if ( d1 && d1->musElement() )
		return ( d1->musElement()->timeEnd() );
	else
		return 0;
}

/*!
 * Helper function for timeToCoords() when doing the binary search over elements.
 * 
 * \sa timeMusElementLessThan
 */
bool CAScoreView::musElementTimeLessThan(const CAMusElement* a, const int b) {
     return (a->timeStart() < b);
}

/*!
 * Helper function for timeToCoords() when doing the binary search over elements.
 * 
 * \sa musElementTimeLessThan
 */
bool CAScoreView::timeMusElementLessThan(const int a, const CAMusElement* b) {
     return (a < b->timeStart());
}


/*!
	Simple Version of \sa timeToCoords( time ):
	Returns the X coordinate for the given Canorus \a time.
	Returns -1, if such a time doesn't exist in the score.
*/
double CAScoreView::timeToCoordsSimpleVersion( int time ) {
	CADrawableMusElement *leftElt = 0;
	
	QList<CAVoice*> voiceList = _sheet->voiceList();
	for (int i=0; i<voiceList.size(); i++) {
		// get the element still smaller or equal, but nearest to time
		QList<CAMusElement*>::const_iterator it = qLowerBound(voiceList[i]->musElementList().constBegin(), voiceList[i]->musElementList().constEnd(), time, CAScoreView::musElementTimeLessThan);
		if (it!=voiceList[i]->musElementList().constEnd()) {
			if (_mapDrawable.contains(*it)) {
				CADrawableMusElement *dElt = static_cast<CADrawableMusElement*>(_mapDrawable.values(*it).last());
				if (leftElt->xPos()<dElt->xPos()) {
					leftElt = dElt;
				}
			} else {
				std::cerr << "ERROR: Drawable instance of musElement " << (*it) << " doesn't exist in _mapDrawable!" << std::endl;
			}
		}
	}
	
	return leftElt?(leftElt->xPos()):(-1);
}

/*!
	Returns the X coordinate for the given Canorus \a time.
	Returns -1, if such a time doesn't exist in the score.
*/
double CAScoreView::timeToCoords( int time ) {
	CADrawableMusElement *leftElt = 0;
	CADrawableMusElement *rightElt = 0;
	
	QList<CAVoice*> voiceList = _sheet->voiceList();
	for (int i=0; i<voiceList.size(); i++) {
		// get the element still smaller or equal, but nearest to time
		QList<CAMusElement*>::const_iterator it = qLowerBound(voiceList[i]->musElementList().constBegin(), voiceList[i]->musElementList().constEnd(), time, CAScoreView::musElementTimeLessThan);
		if (it!=voiceList[i]->musElementList().constEnd()) {
			if (_mapDrawable.contains(*it)) {
				CADrawableMusElement *dElt = static_cast<CADrawableMusElement*>(_mapDrawable.values(*it).last());
				if (!leftElt || leftElt->xPos()<dElt->xPos()) {
					leftElt = dElt;
				}
			} else {
				std::cerr << "ERROR: Drawable instance of musElement " << (*it) << " doesn't exist in _mapDrawable!" << std::endl;
			}
		}
		
		// and for the right element
		it = qUpperBound(voiceList[i]->musElementList().constBegin(), voiceList[i]->musElementList().constEnd(), time, CAScoreView::timeMusElementLessThan);
		if (it!=voiceList[i]->musElementList().constEnd()) {
			if (_mapDrawable.contains(*it)) {
				CADrawableMusElement *dElt = static_cast<CADrawableMusElement*>(_mapDrawable.values(*it).first());
				if (!rightElt || rightElt->xPos()>dElt->xPos()) {
					rightElt = dElt;
				}
			} else {
				std::cerr << "ERROR: Drawable instance of musElement " << (*it) << " doesn't exist in _mapDrawable!" << std::endl;
			}
		}

	}
	
	// get the relative position between the nearest left and the nearest right elements
	if ( leftElt && rightElt && leftElt->musElement() && rightElt->musElement() ) {
		int delta = (rightElt->musElement()->timeStart() - leftElt->musElement()->timeStart());
		if (!delta) delta=1;
		return leftElt->xPos() + ( rightElt->xPos() - leftElt->xPos() ) *
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
	\enum CAScoreView::CAScrollBarVisibility
	Different behaviour of the scroll bars:
		- ScrollBarAlwaysVisible - scrollbars are always visible, no matter if the whole scene can be rendered on canvas or not
		- ScrollBarAlwaysHidden - scrollbars are always hidden, no matter if the whole scene can be rendered on canvas or not
		- ScrollBarShowIfNeeded - scrollbars are visible, if they are needed (the current view area is too small to render the whole
		  scene), otherwise hidden. This is default behaviour.
*/

/*!
	\fn float CAScoreView::zoom()
	Returns the zoom level of the view (1.0 = 100%, 1.5 = 150% etc.).
*/

/*!
	\fn void CAScoreView::setRepaintArea(QRect *area)
	Sets the area to be repainted, not the whole widget.

	\sa clearRepaintArea()
*/

/*!
	\fn void CAScoreView::clearRepaintArea()
	Disables and deletes the area to be repainted.

	\sa setRepaintArea()
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
