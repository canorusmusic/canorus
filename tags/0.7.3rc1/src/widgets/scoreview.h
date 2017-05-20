/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef SCOREVIEW_H_
#define SCOREVIEW_H_

#include <QList>
#include <QPen>
#include <QBrush>
#include <QRect>
#include <QLineEdit>
#include <QTimer>
#include <QMultiMap>

#include "widgets/view.h"
#include "layout/kdtree.h"
#include "score/note.h"

class QScrollBar;
class QMouseEvent;
class QWheelEvent;
class QTimer;
class QGridLayout;

class CADrawable;
class CADrawableMusElement;
class CADrawableContext;
class CADrawableNote;
class CAMusElement;
class CAContext;
class CASheet;
class CAStaff;
class CALyricsContext;
class CADrawableNoteCheckerError;

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

class CAScoreView : public CAView {
Q_OBJECT

public:
	enum CAScrollBarVisibility {
		ScrollBarAlwaysVisible,
		ScrollBarAlwaysHidden,
		ScrollBarShowIfNeeded
	};

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

	////////////////////////////////////////////
	// Addition, removal of drawable elements //
	////////////////////////////////////////////
	void addMElement(CADrawableMusElement *elt, bool select=false);
	void addCElement(CADrawableContext *elt, bool select=false);
	void addDrawableNoteCheckerError(CADrawableNoteCheckerError *dnce);

	void importElements(CAKDTree<CADrawableMusElement*> *drawableMList, CAKDTree<CADrawableContext*> *drawableCList);

	///////////////
	// Selection //
	///////////////
	inline const QList<CADrawableMusElement*>& selection() { return _selection; };
	QList<CAMusElement*>                 musElementSelection();
	QList<CADrawableMusElement*>         musElementsAt(double x, double y);
	CADrawableContext                   *selectCElement(double x, double y);
	CADrawableMusElement                *selectMElement(CAMusElement *elt);
	CADrawableContext                   *selectContext(CAContext *context);
	inline QPoint                        lastMousePressCoords() { return _lastMousePressCoords; }
	void                                 setLastMousePressCoordsAfter(const QList<CAMusElement*> list);

	inline CADrawableContext *currentContext() { return _currentContext; }
	inline void setCurrentContext(CADrawableContext *c) { _currentContext = c; }

	void selectAll();
	void selectAllCurBar();
	void selectAllCurContext();
	void invertSelection();
	inline void clearSelection() { _selection.clear(); emit selectionChanged(); }
	inline bool removeFromSelection(CADrawableMusElement *elt) { return _selection.removeAll(elt); emit selectionChanged(); }

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
	bool mouseDragActivated();

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
	static bool musElementTimeLessThan(const CAMusElement* a, const int b);
	static bool timeMusElementLessThan(const int a, const CAMusElement* b);

	CADrawableContext *nearestUpContext(double x, double y);
	CADrawableContext *nearestDownContext(double x, double y);

	int calculateTime(double x, double y);

	CAContext *contextCollision(double x, double y);

	////////////////
	// Scrollbars //
	////////////////
	inline void setManualScroll(bool scroll) { _allowManualScroll = scroll; }
	inline bool manualScroll() { return _allowManualScroll; }

	void checkScrollBars();

	inline CAScrollBarVisibility isScrollBarVisible() { return _scrollBarVisible;}
	void setScrollBarVisible(CAScrollBarVisibility status);

	//////////////////////////////////////////////
	// Scene appearance, properties and actions //
	//////////////////////////////////////////////
	void rebuild();
	void setMouseTracking(bool); // reimplemented!
	inline const int drawableWidth() { return _canvas->width(); }
	inline const int drawableHeight() { return _canvas->height(); }

	void setWorldX(double x, bool animate=false, bool force=false);
	void setWorldY(double y, bool animate=false, bool force=false);
	void setWorldWidth(double w, bool force=false);
	void setWorldHeight(double h, bool force=false);

	inline const double worldX() { return _worldX; }
	inline const double worldY() { return _worldY; }
	inline const double worldWidth() { return _worldW; }
	inline const double worldHeight() { return _worldH; }
	inline const QRectF worldCoords() { return QRectF(worldX(), worldY(), worldWidth(), worldHeight()); }

	inline const float zoom() { return _zoom; }

	void setWorldCoords(const QRectF r, bool animate=false, bool force=false);
	void setWorldCoords(double x, double y, double w, double h, bool animate=false, bool force=false)  { setWorldCoords( QRect(x,y,w,h), animate, force); }

	void setCenterCoords(double x, double y, bool animate=false, bool force=false);

	void setZoom(float z, double x=0, double y=0, bool animate=false, bool force = false);
	void setZoom(float z, QPoint p,  bool animate=false, bool force = false) { setZoom(z, p.x(), p.y(), animate, force); }

	void zoomToSelection(bool animate=false, bool force=false);
	void zoomToWidth(bool animate=false, bool force=false);
	void zoomToHeight(bool animate=false, bool force=false);
	void zoomToFit(bool animate=false, bool force=false);

	bool grabTabKey() { return _grabTabKey; }
	void setGrabTabKey( bool g ) { _grabTabKey = g; }

	void setBorder(const QPen pen);
	void unsetBorder();
	inline QPen border() { return _borderPen; }
	inline QColor backgroundColor() { return _backgroundColor; }
	inline void setBackgroundColor( const QColor c ) { _backgroundColor = c; }
	inline QColor foregroundColor() { return _foregroundColor; }
	inline void setForegroundColor( const QColor c ) { _foregroundColor = c; }
	inline QColor selectionColor() { return _selectionColor; }
	inline void setSelectionColor( const QColor c ) { _selectionColor = c; }
	inline QColor selectionAreaColor() { return _selectionAreaColor; }
	inline void setSelectionAreaColor( const QColor c ) { _selectionAreaColor = c; }
	inline QColor selectedContextColor() { return _selectedContextColor; }
	inline void setSelectedContextColor( const QColor c ) { _selectedContextColor = c; }
	inline QColor hiddenElementsColor() { return _hiddenElementsColor; }
	inline void setHiddenElementsColor( const QColor c ) { _hiddenElementsColor = c; }
	inline QColor disabledElementsColor() { return _disabledElementsColor; }
	inline void setDisabledElementsColor( const QColor c ) { _disabledElementsColor = c; }

	inline bool playing() { return _playing; }
	inline void setPlaying(bool playing) { _playing = playing; }

	inline void setRepaintArea(QRect *area) { _repaintArea = area; }
	inline void clearRepaintArea() { if (_repaintArea) delete _repaintArea; _repaintArea=0; }

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

	void HScrollBarEvent(int val);
	void VScrollBarEvent(int val);

	void resizeEvent(QResizeEvent *e);
	void paintEvent(QPaintEvent *p);
	void leaveEvent(QEvent *e);
	void enterEvent(QEvent *e);
	void on_animationTimer_timeout();
	void on_clickTimer_timeout();

signals:
	void CATripleClickEvent( QMouseEvent *e, QPoint p );
	void CADoubleClickEvent( QMouseEvent *e, QPoint p );
	void CAMousePressEvent( QMouseEvent *e, QPoint p );
	void CAMouseReleaseEvent( QMouseEvent *e, QPoint p );
	void CAMouseMoveEvent( QMouseEvent *e, QPoint p );
	void CAWheelEvent( QWheelEvent *e, QPoint p );
	void CAKeyPressEvent( QKeyEvent *e );
	void selectionChanged();

protected:
	bool event( QEvent *event );

private:
	void initScoreView( CASheet *s );
	inline void clearMElements() { _drawableMList.clear(true); }
	inline void clearCElements() { _drawableCList.clear(true); }
	inline bool isSelected(CADrawableMusElement *elt) { return (_selection.contains(elt)); }

	//////////////////
	// Core Widgets //
	//////////////////
	QGridLayout *_layout;    // Grid layout for placing the scrollbars at the right and the bottom.
	QWidget *_canvas;        // Virtual canvas which represents the size of the drawable area. All its signals are forwarded to CAView.
	QScrollBar *_hScrollBar, *_vScrollBar; // Horizontal/vertical scrollbars

	////////////////////////
	// General properties //
	////////////////////////
	CAKDTree<CADrawableMusElement*>       _drawableMList;   // The list of music elements stored in a tree for faster lookup and other operations. Every view has its own list of drawable elements and drawable objects themselves!
	CAKDTree<CADrawableContext*>          _drawableCList;   // The list of context drawable elements (staffs, lyrics etc.). Every view has its own list of drawable elements and drawable objects themselves!
	CAKDTree<CADrawableNoteCheckerError*> _drawableNCEList; // The list of drawable note checker errors
	QMultiMap<void*, CADrawable*>         _mapDrawable;     // Mapping of all music elements/contexts in the score -> drawable elements on canvas
	CASheet                              *_sheet;           // Pointer to the CASheet which the view represents.

	QList<CADrawableMusElement *>   _selection;      // The set of elements being selected.
	CADrawableContext              *_currentContext; // The pointer to the currently active context (staff, lyrics).

	static const int RIGHT_EXTRA_SPACE;	  // Extra space at the right end to insert new music
	static const int BOTTOM_EXTRA_SPACE;  // Extra space at the bottom end to insert new music
	static const int RULER_HEIGHT;        // Ruler height in pixels
	template <typename T> double getMaxXExtended(CAKDTree<T> &v);  // Make the viewable World a little bigger (stuffed) to make inserting at the end easier
	template <typename T> double getMaxYExtended(CAKDTree<T> &v);  // Make the viewable World a little bigger (stuffed) to make inserting below easies

	double _worldX, _worldY, _worldW, _worldH;	// Absolute world coordinates of the area the view is currently showing.
	QPoint _lastMousePressCoords;               // Used in multiple selection - coordinates of the upper-left point of the rectangle the user drags in world coordinates
	inline void setLastMousePressCoords( QPoint p ) { _lastMousePressCoords = p; }
	float _zoom;                                // Zoom level of the view (1.0 = 100%, 1.5 = 150% etc.).

	CAVoice *_selectedVoice;        // Voice to be drawn normal colors, others are shaded

	/////////////
	// Helpers //
	/////////////
	// Shadow note
	bool _shadowNoteVisible;        // Should the shadow notes be rendered or not

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
	void drawSelectionRegion( QPainter *p, CADrawSettings s );
public:
	static const int SELECTION_REGION_THRESHOLD; // Threshold in px for mouse move until the selection region is activated
	
private:
	////////////////
	// Appearance //
	////////////////
	bool _grabTabKey;              // Pass the tab key to keyPressEvent() or treat it like the next item key
	bool _drawBorder;              // Should the border be drawn or not.
	QRect *_repaintArea;           // Area to be repainted on paintEvent().
	QPen _borderPen;               // Pen which the border is drawn by.
	QColor _backgroundColor;       // Color which the background is filled.
	QColor _foregroundColor;       // Color which the music elements are painted.
	QColor _selectionColor;        // Color which the selected music elements are painted.
	QColor _selectionAreaColor;    // Color which the selection area background is filled.
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
	float _targetZoom;                // Zoom level of the view (1.0 = 100%, 1.5 = 150% etc.).

	void startAnimationTimer();

	///////////////////////
	// Widgets behaviour //
	///////////////////////
	CAScrollBarVisibility _scrollBarVisible;  // Are the scrollbars always visible/never/if needed. Use CAView::ScrollBarAlwaysVisible, CAView::ScrollBarAlwaysHidden or CAView::ScrollBarShowIfNeeded.
	bool _allowManualScroll; // Does the scrollbars actually react on user actions - sometimes we only want the scrollbars to show the current location of the score and don't do anything

	/////////////////////////
	// Internal properties //
	/////////////////////////
	double _oldWorldX, _oldWorldY, _oldWorldW, _oldWorldH; // Old coordinates used before the repaint. This is needed so only the new part of the view gets repainted when panning.
	bool _playing;                                      // Set to on, when in Playback mode
	QTimer *_clickTimer;                                // Used for measuring doubleClick and tripleClick
	int     _numberOfClicks;                            // Used for measuring doubleClick and tripleClick

	double _xCursor, _yCursor;                             // Mouse cursor position in absolute world coords.
	bool _holdRepaint;                                  // Flag to prevent multiple repaintings.
	bool _checkScrollBarsDeadLock;                      // Flag to prevent recursive checkScrollBars() calls.
	bool _hScrollBarDeadLock;                           // Flag to prevent recursive scrollbar calls when its value is manually changed.
	bool _vScrollBarDeadLock;                           // Flag to prevent recursive scrollbar calls when its value is manually changed.
};

#endif /*SCOREVIEW_H_*/
