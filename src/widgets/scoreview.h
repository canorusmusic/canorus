/*!
	Copyright (c) 2006-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef SCOREVIEW_H_
#define SCOREVIEW_H_

#include <QGraphicsView>
#include <QGraphicsScene>

#include <QList>
#include <QPen>
#include <QBrush>
#include <QRect>
#include <QLineEdit>
#include <QTimer>

#include "widgets/view.h"
#include "layout/kdtree.h"
#include "score/note.h"

class QMouseEvent;
class QWheelEvent;
class QTimer;
class QLayout;

class CALayoutEngine;
class CADrawable;
class CADrawableMusElement;
class CADrawableContext;
class CADrawableNote;
class CAMusElement;
class CAContext;
class CASheet;
class CAStaff;
class CALyricsContext;

class CATextEdit : public QLineEdit {
Q_OBJECT

public:
	CATextEdit( QWidget *parent=0 );
	~CATextEdit();

signals:
	void CAKeyPressEvent( QKeyEvent * );

public slots:
	void keyPressEvent( QKeyEvent * );
};

class CAGraphicsView : public QGraphicsView {
Q_OBJECT

public:
	CAGraphicsView( QWidget *parent=0 );

public slots:
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseDoubleClickEvent(QMouseEvent *e);
	void wheelEvent(QWheelEvent *e);
	void keyPressEvent(QKeyEvent *e);
};

class CAScoreView : public CAView {
Q_OBJECT

public:
	///////////////////
	// Basic methods //
	///////////////////
	CAScoreView(QWidget *parent=0);
	CAScoreView(CASheet *sheet, QWidget *parent=0);
	virtual ~CAScoreView();
	CAScoreView *clone();
	CAScoreView *clone(QWidget *parent);
	inline CASheet  *sheet() { return _sheet; }
	inline void setSheet( CASheet *sheet ) { _sheet = sheet; }
	inline QGraphicsView *canvas() { return _canvas; }
	inline QGraphicsScene *scene() { return _scene; }

	////////////////////////////////////////////
	// Addition, removal of drawable elements //
	////////////////////////////////////////////
	void addMElement(CADrawableMusElement *elt, bool select=false);
	void addCElement(CADrawableContext *elt, bool select=false);
	CAMusElement *removeMElement(double x, double y);

	///////////////
	// Selection //
	///////////////
	inline const QList<CADrawableMusElement*>& selection() { return _selection; };
	QList<CAMusElement*>                 musElementSelection();
	QList<CADrawableMusElement*>         musElementsAt(const QPointF& p);
	CADrawableContext                   *selectCElement(const QPointF& p);
	CADrawableMusElement                *selectMElement(CAMusElement *elt);
	CADrawableContext                   *selectContext(CAContext *context);
	inline QPointF                       lastMouseMoveCoords() { return _lastMouseMoveCoords; }
	inline QPointF                       lastMousePressCoords() { return _lastMousePressCoords; }
	void                                 setLastMousePressCoordsAfter(const QList<CAMusElement*> list);

	inline CADrawableContext *currentContext() { return _currentContext; }
	inline void setCurrentContext(CADrawableContext *c) { _currentContext = c; }

	void selectAll();
	void selectAllCurBar();
	void selectAllCurContext();
	void invertSelection();
	void clearSelection();
	bool removeFromSelection(CADrawableMusElement *elt);

	void                  addToSelection(CADrawableMusElement *elt, bool triggerSignal=true );
	void                  addToSelection( const QList<CADrawableMusElement*> list, bool selectableOnly=true );
	CADrawableMusElement *addToSelection(CAMusElement *elt);
	void                  addToSelection(const QList<CAMusElement *> elts);

	CADrawableMusElement* selectNextMusElement( bool append=false );
	CADrawableMusElement* selectPrevMusElement( bool append=false );
	CADrawableMusElement* selectUpMusElement();
	CADrawableMusElement* selectDownMusElement();

	inline const QList<QRect> &selectionRegionList() const { return _selectionRegionList; }
	inline void addSelectionRegion(QRect r) { _selectionRegionList << r; }
	inline void removeSelectionRegion(QRect r) { _selectionRegionList.removeAll(r); }
	inline void clearSelectionRegionList() { _selectionRegionList.clear(); }
	inline CADrawable::CADirection resizeDirection() { return _resizeDirection; }

	/////////////////////////////////////////////////////////////////////
	// Music elements and contexts query, space calculation and access //
	/////////////////////////////////////////////////////////////////////
	CADrawableMusElement     *findMElement(CAMusElement*);
	CADrawableContext        *findCElement(CAContext*);
	QList<CADrawableContext*> findContextsInRegion(QRect &reg);
	CADrawableMusElement     *nearestLeftElement(double x, double y, CADrawableContext* context=0);
	CADrawableMusElement     *nearestLeftElement(double x, double y, CAVoice *voice);
	CADrawableMusElement     *nearestRightElement(double x, double y, CADrawableContext* context=0);
	CADrawableMusElement     *nearestRightElement(double x, double y, CAVoice *voice);
	int coordsToTime( double x );
	double timeToCoords( int time );
	double timeToCoordsSimpleVersion( int time );

	CADrawableContext *nearestUpContext(double x, double y);
	CADrawableContext *nearestDownContext(double x, double y);

	int calculateTime(double x, double y);

	CAContext *contextCollision(double x, double y);

	////////////////
	// Scrollbars //
	////////////////
	Qt::ScrollBarPolicy scrollBarPolicy();
	void setScrollBarPolicy(Qt::ScrollBarPolicy);

	//////////////////////////////////////////////
	// Scene appearance, properties and actions //
	//////////////////////////////////////////////
	void rebuild();
	bool isInsideCanvas(QPointF&);

	inline const double zoom() { return _canvas->width()/sceneRect().width(); }

	inline const QRectF sceneRect() { return _canvas->mapToScene(0, 0, _canvas->width(), _canvas->height()).boundingRect(); }
	void setSceneRect(const QRectF& r, bool animate=false);
	void setSceneRect(double x, double y, double w, double h, bool animate=false)  { setSceneRect( QRectF(x,y,w,h), animate); }

	double sceneX() { return sceneRect().x(); }
	double sceneY() { return sceneRect().y(); }
	double sceneWidth() { return sceneRect().width(); }
	double sceneHeight() { return sceneRect().height(); }

	void setSceneX(double x, bool animate=false) { setSceneRect( QRectF(x, sceneRect().y(), sceneRect().width(), sceneRect().height()), animate ); }
	void setSceneY(double y, bool animate=false) { setSceneRect( QRectF(sceneRect().x(), y, sceneRect().width(), sceneRect().height()), animate ); }
	void setSceneWidth(double w, bool animate=false) { setSceneRect( QRectF(sceneRect().x(), sceneRect().y(), w, sceneRect().height()), animate ); }
	void setSceneHeight(double h, bool animate=false) { setSceneRect( QRectF(sceneRect().x(), sceneRect().x(), sceneRect().width(), h), animate ); }

	void centerOn(double x, double y, bool animate=false);

	void setZoom(double z, double x=0, double y=0, bool animate=false);
	void setZoom(double z, QPointF p,  bool animate=false) { setZoom(z, p.x(), p.y(), animate); }

	void zoomToSelection(bool animate=false);
	void zoomToWidth(bool animate=false);
	void zoomToHeight(bool animate=false);
	void zoomToFit(bool animate=false);

	bool grabTabKey() { return _grabTabKey; }
	void setGrabTabKey( bool g ) { _grabTabKey = g; }

	void setBorder(const QPen pen);
	void unsetBorder();
	inline QPen border() { return _borderPen; }
	inline QBrush backgroundBrush() { return _canvas->backgroundBrush(); }
	inline void setBackgroundBrush( const QBrush c ) { _canvas->setBackgroundBrush(c); }
	inline QColor foregroundColor() { return _foregroundColor; }
	inline void setForegroundColor( const QColor& c ) { _foregroundColor = c; }
	inline QColor selectionColor() { return _selectionColor; }
	inline void setSelectionColor( const QColor& c ) { _selectionColor = c; }
	inline QBrush selectionAreaBrush() { return _selectionAreaBrush; }
	inline void setSelectionAreaBrush( const QBrush c ) { _selectionAreaBrush = c; }
	inline QColor selectedContextColor() { return _selectedContextColor; }
	inline void setSelectedContextColor( const QColor& c ) { _selectedContextColor = c; }
	inline QColor hiddenElementsColor() { return _hiddenElementsColor; }
	inline void setHiddenElementsColor( const QColor& c ) { _hiddenElementsColor = c; }
	inline QColor disabledElementsColor() { return _disabledElementsColor; }
	inline void setDisabledElementsColor( const QColor& c ) { _disabledElementsColor = c; }

	inline bool playing() { return _playing; }
	inline void setPlaying(bool playing) { _playing = playing; }

	inline CAVoice *selectedVoice() { return _selectedVoice; }
	inline void setSelectedVoice( CAVoice *selectedVoice ) { _selectedVoice = selectedVoice; }

	inline bool shadowNoteVisible() { return _shadowNoteVisible; }
	inline void setShadowNoteVisible(bool visible) { _shadowNoteVisible = visible; setShadowNoteVisibleOnLeave(visible); }

	inline bool drawShadowNoteAccs() { return _drawShadowNoteAccs; }
	inline void setDrawShadowNoteAccs(bool draw) { _drawShadowNoteAccs = draw; }

	inline int shadowNoteAccs() { return _shadowNoteAccs; }
	inline void setShadowNoteAccs(int accs) { _shadowNoteAccs = accs; }

	void setShadowNoteLength( CAPlayableLength );

	CATextEdit *createTextEdit( CADrawableMusElement *elt );
	inline CATextEdit *textEdit() { return _textEdit; }
	void removeTextEdit();
	inline bool textEditVisible() { return _textEditVisible; }

	bool noteNameVisible() { return _noteNameVisible; }
	void setNoteNameVisible( bool v ) { _noteNameVisible = v; }

	QString noteName() { return _noteName; }
	void setNoteName( QString n ) { _noteName = n; }

	void updateHelpers(); // method for updating shadow notes, syllable edits and other post-engrave elements coordinates and sizes when zoom level is changed etc.

private slots:
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void wheelEvent(QWheelEvent *e);
	void keyPressEvent(QKeyEvent *e);

	void leaveEvent(QEvent *e);
	void enterEvent(QEvent *e);
	void on_animationTimer_timeout();
	void on_clickTimer_timeout();

signals:
	void CATripleClickEvent( QMouseEvent *e, QPointF p );
	void CADoubleClickEvent( QMouseEvent *e, QPointF p );
	void CAMousePressEvent( QMouseEvent *e, QPointF p );
	void CAMouseReleaseEvent( QMouseEvent *e, QPointF p );
	void CAMouseMoveEvent( QMouseEvent *e, QPointF p );
	void CAWheelEvent( QWheelEvent *e, QPointF p );
	void CAKeyPressEvent( QKeyEvent *e );
	void selectionChanged();

protected:
	bool event( QEvent *event );

private:
	void initScoreView( CASheet *s );
	inline void clearMElements() { _drawableMList.clear(true); }
	inline void clearCElements() { _drawableCList.clear(true); }
	inline bool isSelected(CADrawableMusElement *elt) { return (_selection.contains(elt)); }

	/////////////
	// Widgets //
	/////////////
	QLayout        *_layout; // Layout for automatic resize of contents
	QGraphicsView  *_canvas; // Virtual canvas which represents the size of the drawable area. All its signals are forwarded to CAView.
	QGraphicsScene *_scene;  // Virtual scene which contains the graphical elements generated by the engraver

	////////////////////////
	// General properties //
	////////////////////////
	CAKDTree<CADrawableMusElement*> _drawableMList;  // The list of music elements stored in a tree for faster lookup and other operations. Every view has its own list of drawable elements and drawable objects themselves!
	CAKDTree<CADrawableContext*>    _drawableCList;  // The list of context drawable elements (staffs, lyrics etc.). Every view has its own list of drawable elements and drawable objects themselves!
	CASheet                        *_sheet;          // Pointer to the CASheet which the view represents.
	CALayoutEngine                 *_layoutEngine;   // Instance of the layout engine

	QList<CADrawableMusElement *>   _selection;      // The set of elements being selected.
	CADrawableContext              *_currentContext; // The pointer to the currently active context (staff, lyrics).

	static const int RIGHT_EXTRA_SPACE;	  // Extra space at the right end to insert new music
	static const int BOTTOM_EXTRA_SPACE;  // Extra space at the bottom end to insert new music
	int getMaxXExtended();                // Make the viewable World a little bigger (stuffed) to make inserting at the end easier
	int getMaxYExtended();                // Make the viewable World a little bigger (stuffed) to make inserting below easies

	QPointF _lastMousePressCoords;        // Used in multiple selection - coordinates of the upper-left point of the rectangle the user drags in world coordinates
	inline void setLastMousePressCoords( QPointF p ) { _lastMousePressCoords = p; }

	QPointF _lastMouseMoveCoords;         // Used when updating helpers
	inline void setLastMouseMoveCoords( QPointF p ) { _lastMouseMoveCoords = p; }

	CAVoice *_selectedVoice;        // Voice to be drawn normal colors, others are shaded

	/////////////
	// Helpers //
	/////////////
	// Shadow note
	bool _shadowNoteVisible;            // Should the shadow notes be rendered or not
        QGraphicsTextItem *_shadowNoteName; // Text showing the current shadow note pitch

	bool _shadowNoteVisibleOnLeave; // When you leave the view, shadow note is always turned off. This property holds the value, if shadow note was enabled before you left the view.
	inline bool shadowNoteVisibleOnLeave() { return _shadowNoteVisibleOnLeave; }
	inline void setShadowNoteVisibleOnLeave( bool v ) { _shadowNoteVisibleOnLeave = v; }

	int _shadowNoteAccs;            // Number of accidentals - 0 - natural, 1 - sharp, -1 flat
	bool _drawShadowNoteAccs;       // Draw shadow note accs?
	QList<CANote*> _shadowNote;     // List of all shadow notes - one shadow note per drawable staff
	QList<CADrawableNote*> _shadowDrawableNote;	// List of drawable shadow notes

	// QLineEdit for editing or creating a lyrics syllable
	CATextEdit *_textEdit;
	inline void setTextEdit( CATextEdit *e ) { _textEdit = e; }
	QRect _textEditGeometry;
	inline QRect textEditGeometry() { return _textEditGeometry; }
	inline void setTextEditGeometry( const QRect r ) { _textEditGeometry = r; }
	bool _textEditVisible;
	inline void setTextEditVisible(bool v) { _textEditVisible = v; }
	inline void setResizeDirection( CADrawable::CADirection r ) { _resizeDirection = r; }
	CADrawable::CADirection _resizeDirection; // Is the current scalable music element in drag-drop resizing mode?

	// Selection regions
	QList<QRect> _selectionRegionList;
/*	void drawSelectionRegion( QPainter *p, CADrawSettings s );
*/
	////////////////
	// Appearance //
	////////////////
	bool _grabTabKey;              // Pass the tab key to keyPressEvent() or treat it like the next item key
	bool _drawBorder;              // Should the border be drawn or not.
	QPen _borderPen;               // Pen which the border is drawn by.
	QColor _foregroundColor;       // Color which the normal music elements are painted.
	QColor _selectionColor;        // Color which the selected music elements are painted.
	QBrush _selectionAreaBrush;    // Color which the selection area background is filled.
	QColor _selectedContextColor;  // Color which the current context is painted.
	QColor _disabledElementsColor; // Color which the elements in non-selected voice are painted.
	QColor _hiddenElementsColor;   // Color which the invisible elements are painted in current-voice-only mode.
	bool    _noteNameVisible;      // Is the written note name visible
	QString _noteName;             // Name of the note to be inserted. eg. c', Des,

	///////////////
	// Animation //
	///////////////
	QTimer *_animationTimer;          // Timer used to animate scroll/zoom behaviour.
	static const int ANIMATION_STEPS; // Number of steps used in animation
	int _animationStep;               // Current step in the animation
	double _targetWorldX, _targetWorldY, _targetWorldW, _targetWorldH;	// Absolute world coordinates of the area the view is currently showing.
	double _targetZoom;                // Zoom level of the view (1.0 = 100%, 1.5 = 150% etc.).

	void startAnimationTimer();

	/////////////////////////
	// Internal properties //
	/////////////////////////
	bool _playing;                                      // Set to on, when in Playback mode
	QTimer *_clickTimer;                                // Used for measuring doubleClick and tripleClick
	int     _numberOfClicks;                            // Used for measuring doubleClick and tripleClick
};

#endif /*SCOREVIEW_H_*/
