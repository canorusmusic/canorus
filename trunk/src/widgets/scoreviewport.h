/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef SCOREVIEWPORT_H_
#define SCOREVIEWPORT_H_

#include <QList>
#include <QPen>
#include <QBrush>
#include <QRect>
#include <QLineEdit>

#include "widgets/viewport.h"
#include "core/kdtree.h"
#include "core/note.h"

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

class CASyllableEdit : public QLineEdit {
Q_OBJECT

public:
	CASyllableEdit( QWidget *parent=0 );
	~CASyllableEdit();
	
signals:
	void CAKeyPressEvent( QKeyEvent *, CASyllableEdit * );
	
public slots:
	void keyPressEvent( QKeyEvent * );
};

class CAScoreViewPort : public CAViewPort {
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
	CAScoreViewPort(CASheet *sheet, QWidget *parent);
	~CAScoreViewPort();
	CAScoreViewPort *clone();
	CAScoreViewPort *clone(QWidget *parent);
	inline CASheet  *sheet() { return _sheet; }
	
	////////////////////////////////////////////
	// Addition, removal of drawable elements //
	////////////////////////////////////////////
	void addMElement(CADrawableMusElement *elt, bool select=false);
	void addCElement(CADrawableContext *elt, bool select=false);
	CAMusElement *removeMElement(int x, int y);
	
	void importElements(CAKDTree<CADrawableMusElement*> *drawableMList, CAKDTree<CADrawableContext*> *drawableCList);
	void importMElements(CAKDTree<CADrawableMusElement*> *elts);
	void importCElements(CAKDTree<CADrawableContext*> *elts);
	
	///////////////
	// Selection //
	///////////////
	inline QList<CADrawableMusElement*> selection() { return _selection; };
	QList<CADrawableMusElement*>        musElementsAt(int x, int y);
	CADrawableContext                  *selectCElement(int x, int y);
	CADrawableMusElement               *selectMElement(CAMusElement *elt);
	CADrawableContext                  *selectContext(CAContext *context);
	inline QPoint                       lastMousePressCoords() { return _lastMousePressCoords; }
	
	inline CADrawableContext *currentContext() { return _currentContext; }
	inline void setCurrentContext(CADrawableContext *c) { _currentContext = c; }
	
	inline void clearSelection() { _selection.clear(); }
	inline bool removeFromSelection(CADrawableMusElement *elt) { return _selection.removeAll(elt); }
	
	inline void           addToSelection(CADrawableMusElement *elt) {
		int i;
		for (i=0; i<_selection.size() && _selection[i]->xPos() < elt->xPos(); i++);
		_selection.insert( i, elt );
	}
	inline void           addToSelection(const QList<CADrawableMusElement*> list) { _selection << list; }
	CADrawableMusElement *addToSelection(CAMusElement *elt);
	void                  addToSelection(const QList<CAMusElement *> elts);
	
	CADrawableMusElement* selectNextMusElement();
	CADrawableMusElement* selectPrevMusElement();
	CADrawableMusElement* selectUpMusElement();
	CADrawableMusElement* selectDownMusElement();
	
	inline const QList<QRect> &selectionRegionList() const { return _selectionRegionList; }
	inline void addSelectionRegion(QRect r) { _selectionRegionList << r; }
	inline void removeSelectionRegion(QRect r) { _selectionRegionList.removeAll(r); }
	inline void clearSelectionRegionList() { _selectionRegionList.clear(); }
	
	/////////////////////////////////////////////////////////////////////
	// Music elements and contexts query, space calculation and access //
	/////////////////////////////////////////////////////////////////////
	CADrawableMusElement     *findMElement(CAMusElement*);
	CADrawableContext        *findCElement(CAContext*);
	QList<CADrawableContext*> findContextsInRegion(QRect &reg);
	CADrawableMusElement     *nearestLeftElement(int x, int y, bool currentContextOnly=true);
	CADrawableMusElement     *nearestLeftElement(int x, int y, CAVoice *voice);
	CADrawableMusElement     *nearestRightElement(int x, int y, bool currentContextOnly=true);
	CADrawableMusElement     *nearestRightElement(int x, int y, CAVoice *voice);
	
	CADrawableContext *nearestUpContext(int x, int y);
	CADrawableContext *nearestDownContext(int x, int y);
	
	int calculateTime(int x, int y);
	
	CAContext *contextCollision(int x, int y);
	
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
	inline const int drawableWidth() { return _canvas->width(); }
	inline const int drawableHeight() { return _canvas->height(); }
	
	void setWorldX(int x, bool animate=false, bool force=false);
	void setWorldY(int y, bool animate=false, bool force=false);
	void setWorldWidth(int w, bool force=false);
	void setWorldHeight(int h, bool force=false);
	
	inline const int worldX() { return _worldX; }
	inline const int worldY() { return _worldY; }
	inline const int worldWidth() { return _worldW; }
	inline const int worldHeight() { return _worldH; }
	inline const QRect worldCoords() { return QRect(worldX(), worldY(), worldWidth(), worldHeight()); }

	inline const float zoom() { return _zoom; }
		
	void setWorldCoords(const QRect r, bool animate=false, bool force=false);
	void setWorldCoords(int x, int y, int w, int h, bool animate=false, bool force=false)  { setWorldCoords( QRect(x,y,w,h), animate, force); }
	
	void setCenterCoords(int x, int y, bool animate=false, bool force=false);
	
	void setZoom(float z, int x=0, int y=0, bool animate=false, bool force = false);
	void setZoom(float z, QPoint p,  bool animate=false, bool force = false) { setZoom(z, p.x(), p.y(), animate, force); }
	
	void zoomToSelection(bool animate=false, bool force=false);
	void zoomToWidth(bool animate=false, bool force=false);
	void zoomToHeight(bool animate=false, bool force=false);
	void zoomToFit(bool animate=false, bool force=false);
	
	void setBorder(const QPen pen);
	void setBackground(const QBrush brush);
	void unsetBorder();
	
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
	
	inline void setShadowNoteDotted(int dotted) { for (int i=0; i<_shadowNote.size(); i++) _shadowNote[i]->setDotted(dotted); }
	
	CASyllableEdit *createSyllableEdit( CADrawableMusElement *syllable );
	inline CASyllableEdit *syllableEdit() { return _syllableEdit; }
	void removeSyllableEdit();
	inline bool syllableEditVisible() { return _syllableEditVisible; }
	
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
	void on__animationTimer_timeout();
	
signals:
	void CAMousePressEvent(QMouseEvent *e, QPoint p, CAScoreViewPort *v);
	void CAMouseReleaseEvent(QMouseEvent *e, QPoint p, CAScoreViewPort *v);
	void CAMouseMoveEvent(QMouseEvent *e, QPoint p, CAScoreViewPort *v);
	void CAWheelEvent(QWheelEvent *e, QPoint p, CAScoreViewPort *v);
	void CAKeyPressEvent(QKeyEvent *e, CAScoreViewPort *v);

private:
	inline void clearMElements() { _drawableMList.clear(true); }
	inline void clearCElements() { _drawableCList.clear(true); }
	inline bool isSelected(CADrawableMusElement *elt) { return (_selection.contains(elt)); }
	
	////////////////////////
	// General properties //
	////////////////////////
	CAKDTree<CADrawableMusElement*> _drawableMList;  // The list of music elements stored in a tree for faster lookup and other operations. Every viewport has its own list of drawable elements and drawable objects themselves!
	CAKDTree<CADrawableContext*>    _drawableCList;  // The list of context drawable elements (staffs, lyrics etc.). Every viewport has its own list of drawable elements and drawable objects themselves!
	CASheet                        *_sheet;          // Pointer to the CASheet which the viewport represents.
	
	QList<CADrawableMusElement *>   _selection;      // The set of elements being selected.
	CADrawableContext              *_currentContext; // The pointer to the currently active context (staff, lyrics).
	
	static const int RIGHT_EXTRA_SPACE;	  // Extra space at the right end to insert new music
	static const int BOTTOM_EXTRA_SPACE;  // Extra space at the bottom end to insert new music
	template <typename T> int getMaxXExtended(CAKDTree<T> &v);  // Make the viewable World a little bigger (stuffed) to make inserting at the end easier
	template <typename T> int getMaxYExtended(CAKDTree<T> &v);  // Make the viewable World a little bigger (stuffed) to make inserting below easies
	
	int _worldX, _worldY, _worldW, _worldH;	// Absolute world coordinates of the area the viewport is currently showing.
	QPoint _lastMousePressCoords;           // Used in multiple selection - coordinates of the upper-left point of the rectangle the user drags in world coordinates
	inline void setLastMousePressCoords( QPoint p ) { _lastMousePressCoords = p; }
	float _zoom;                            // Zoom level of the viewport (1.0 = 100%, 1.5 = 150% etc.).
	
	CAVoice *_selectedVoice;        // Voice to be drawn normal colors, others are shaded
	
	/////////////
	// Helpers //
	/////////////
	// Shadow note
	bool _shadowNoteVisible;        // Should the shadow notes be rendered or not
	
	bool _shadowNoteVisibleOnLeave; // When you leave the viewport, shadow note is always turned off. This property holds the value, if shadow note was enabled before you left the viewport.
	inline bool shadowNoteVisibleOnLeave() { return _shadowNoteVisibleOnLeave; }
	inline void setShadowNoteVisibleOnLeave( bool v ) { _shadowNoteVisibleOnLeave = v; }
	
	int _shadowNoteAccs;            // Number of accidentals - 0 - natural, 1 - sharp, -1 flat
	bool _drawShadowNoteAccs;       // Draw shadow note accs?
	QList<CANote*> _shadowNote;     // List of all shadow notes - one shadow note per drawable staff
	QList<CADrawableNote*> _shadowDrawableNote;	// List of drawable shadow notes
	
	// QLineEdit for editing or creating a lyrics syllable
	CASyllableEdit *_syllableEdit;
	inline void setSyllableEdit( CASyllableEdit *e ) { _syllableEdit = e; }
	QRect _syllableEditGeometry;
	inline QRect syllableEditGeometry() { return _syllableEditGeometry; }
	inline void setSyllableEditGeometry( const QRect r ) { _syllableEditGeometry = r; }
	bool _syllableEditVisible;
	inline void setSyllableEditVisible(bool v) { _syllableEditVisible = v; }
	
	// Selection regions
	QList<QRect> _selectionRegionList;
	void drawSelectionRegion( QPainter *p, CADrawSettings s );
	
	////////////////////////////
	// Widgets and appearance //
	////////////////////////////
	QGridLayout *_layout;    // Grid layout for placing the scrollbars at the right and the bottom.
	QWidget *_canvas;        // Virtual canvas which represents the size of the drawable area. All its signals are forwarded to CAViewPort.
	QScrollBar *_hScrollBar, *_vScrollBar; // Horizontal/vertical scrollbars
	bool _drawBorder;        // Should the border be drawn or not.
	QPen _borderPen;         // Pen which the border is drawn by.
	QBrush _backgroundBrush; // Brush which the background is drawn by.
	QRect *_repaintArea;     // Area to be repainted on paintEvent().
	
	///////////////
	// Animation //
	///////////////
	QTimer *_animationTimer;          // Timer used to animate scroll/zoom behaviour.
	static const int ANIMATION_STEPS; // Number of steps used in animation
	int _animationStep;               // Current step in the animation
	int _targetWorldX, _targetWorldY, _targetWorldW, _targetWorldH;	// Absolute world coordinates of the area the viewport is currently showing.
	float _targetZoom;                // Zoom level of the viewport (1.0 = 100%, 1.5 = 150% etc.).
	
	void startAnimationTimer();
	
	///////////////////////
	// Widgets behaviour //
	///////////////////////
	CAScrollBarVisibility _scrollBarVisible;  // Are the scrollbars always visible/never/if needed. Use CAViewPort::ScrollBarAlwaysVisible, CAViewPort::ScrollBarAlwaysHidden or CAViewPort::ScrollBarShowIfNeeded.
	bool _allowManualScroll; // Does the scrollbars actually react on user actions - sometimes we only want the scrollbars to show the current location of the score and don't do anything
	
	/////////////////////////
	// Internal properties //
	/////////////////////////
	int _oldWorldX, _oldWorldY, _oldWorldW, _oldWorldH; // Old coordinates used before the repaint. This is needed so only the new part of the viewport gets repainted when panning.
	bool _playing;                                      // Set to on, when in Playback mode
	
	int _xCursor, _yCursor;                             // Mouse cursor position in absolute world coords.
	bool _holdRepaint;                                  // Flag to prevent multiple repaintings.
	bool _checkScrollBarsDeadLock;                      // Flag to prevent recursive checkScrollBars() calls.
	bool _hScrollBarDeadLock;                           // Flag to prevent recursive scrollbar calls when its value is manually changed.
	bool _vScrollBarDeadLock;                           // Flag to prevent recursive scrollbar calls when its value is manually changed.
};

#endif /*SCOREVIEWPORT_H_*/
