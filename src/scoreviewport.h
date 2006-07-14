/** @file scoreviewport.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef SCOREVIEWPORT_H_
#define SCOREVIEWPORT_H_

#include <QList>
#include <QPen>
#include <QBrush>

#include "viewport.h"
#include "kdtree.h"
#include "note.h"

class QGridLayout;
class QScrollBar;
class QMouseEvent;
class QWheelEvent;

class CADrawable;
class CADrawableMusElement;
class CADrawableContext;
class CAMusElement;
class CAContext;
class CASheet;
class CAStaff;

class CAScoreViewPort : public CAViewPort {
	Q_OBJECT
	
	public:
		CAScoreViewPort(CASheet *sheet, QWidget *parent);
		~CAScoreViewPort();
		
		////////////////////////////////////////////////
		//General methods
		////////////////////////////////////////////////
		CAScoreViewPort *clone();
		CAScoreViewPort *clone(QWidget *parent);
		
		////////////////////////////////////////////////
		//Score opertaions
		////////////////////////////////////////////////
		/**
		 * Return the pointer to the viewport's sheet.
		 * 
		 * @return Pointer to the sheet the viewport represents.
		 */
		CASheet *sheet() { return _sheet; }
		
		void addMElement(CADrawableMusElement *elt, bool select=false);
		void addCElement(CADrawableContext *elt, bool select=false);
		CANote *addNote(CANote::CANoteLength, int x, int y);
		CAMusElement *removeMElement(int x, int y);
		CAMusElement *selectMElement(int x, int y);
		bool selectMElement(CAMusElement *elt);
		CAContext *selectCElement(int x, int y);
		bool selectContext(CAContext *context);		///Returns true, if the context existed and was selected, otherwise false
		
		/**
		 * Returns the nearest left element from the given position with the largest startTime.
		 * 
		 * @param x X coordinate in absolute world units.
		 * @param y Y coordinate in absolute world units.
		 * @param currentContextOnly Only look for the nearest right element in the current drawable context.
		 * @return Pointer to the nearest left element from these world units.
		 */
		CAMusElement *nearestLeftElement(int x, int y, bool currentContextOnly=true);
		
		/**
		 * Returns the nearest right element from the given position with the smallest startTime.
		 * 
		 * @param x X coordinate in absolute world units.
		 * @param y Y coordinate in absolute world units.
		 * @param currentContextOnly Only look for the nearest right element in the current drawable context.
		 * @return Pointer to the nearest rightelement from these world units.
		 */
		CAMusElement *nearestRightElement(int x, int y, bool currentContextOnly=true);

		/**
		 * Calculates the logical time from the given X and Y coordinates.
		 * 
		 * @return Logical time calculated from the given X and Y coordinates.
		 */
		int calculateTime(int x, int y);
		
		/**
		 * If the given coordinates hit any of the contexts, return that context.
		 * 
		 * @return Pointer to the CAContext which collided with the given coordinates.
		 */
		CAContext *contextCollision(int x, int y);
		
		void clearMElements() { _drawableMList.clear(true); }
		void clearCElements() { _drawableCList.clear(true); }
		
		void importMElements(CAKDTree *elts);
		void importCElements(CAKDTree *elts);
		
		/**
		 * Synchronize the sheets music elements with the actual drawable ones and reposition them.
		 */
		void update();
		
		///////////////////////////////////////////////
		//Widget properties
		///////////////////////////////////////////////
		/**
		 * Return the drawable are width.
		 * 
		 * @return Drawable area width in pixels.
		 */
		inline const int drawableWidth() { return _canvas->width(); }
		
		/**
		 * Return the drawable are height.
		 * 
		 * @return Drawable area height in pixels.
		 */
		inline const int drawableHeight() { return _canvas->height(); }

		////////////////////////////////////////////////
		//Scrollbars
		////////////////////////////////////////////////
		/**
		 * Set the manual scroll property.
		 * This means, does the user reaction with scrollbars trigger the scroll of the viewport or not.
		 * 
		 * @param scroll Manual scroll property value.
		 */
		void setManualScroll(bool scroll) {_allowManualScroll = scroll;}
		
		/**
		 * Get the manual scroll property.
		 * This means, does the user reaction with scrollbars trigger the scroll of the viewport or not.
		 * 
		 * @return Manual scroll property value.
		 */
		bool manualScroll() { return _allowManualScroll; }
		
		/**
		 * Check whether the scrollbars are needed (the whole scene is not rendered) or not.
		 * Scrollbars get shown or hidden.
		 * 
		 * Repaint is done automatically, if needed!
		 */
		void checkScrollBars();
		
		/**
		 * Return the scrollbars visibility behaviour.
		 * CAViewPort::ScrollBarAlwaysVisible - scrollbars are always visible, no matter if the whole scene can be rendered on canvas or not
		 * CAViewPort::ScrollBarAlwaysHidden - scrollbars are always hidden, no matter if the whole scene can be rendered on canvas or not
		 * CAViewPort::ScrollBarShowIfNeeded - scrollbars are visible, if they are needed (the current viewport area is too small to render the whole scene), otherwise hidden. This is default behaviour.
		 * 
		 * @return Scrollbars visibility behaviour CAViewPort::ScrollBarAlwaysVisible, CAViewPort::ScrollBarAlwaysHidden or CAViewPort::ScrollBarShowIfNeeded.
		 */
		char scrollBarsVisibility() { return _scrollBarsVisible;}
		
		/**
		 * Set the behaviour of the scrollbars visibility.
		 * CAViewPort::ScrollBarAlwaysVisible - scrollbars are always visible, no matter if the whole scene can be rendered on canvas or not
		 * CAViewPort::ScrollBarAlwaysHidden - scrollbars are always hidden, no matter if the whole scene can be rendered on canvas or not
		 * CAViewPort::ScrollBarShowIfNeeded - scrollbars are visible, if they are needed (the current viewport area is too small to render the whole scene), otherwise hidden. This is default behaviour.
		 * 
		 * @param status Scrollbars visibility behaviour: use CAViewPort::ScrollBarAlwaysVisible, CAViewPort::ScrollBarAlwaysHidden or CAViewPort::ScrollBarShowIfNeeded.
		 */
		void setScrollBarsVisibility(char status);
		enum {ScrollBarAlwaysVisible, ScrollBarAlwaysHidden, ScrollBarShowIfNeeded};

		////////////////////////////////////////////////
		//View and appearance actions and settings
		////////////////////////////////////////////////
		/**
		 * Set the world X coordinate of the viewport.
		 * WARNING! Repaint is not done automatically!
		 * 
		 * @param x Top-left X coordinate of the new viewport area in absolute world units.
		 * @param force Use the given world units despite their illegal values (like negative coordinates etc.).
		 */
		void setWorldX(int x, bool force=false);
		
		/**
		 * Set the world Y coordinate of the viewport.
		 * WARNING! Repaint is not done automatically!
		 * 
		 * @param y Top-left Y coordinate of the new viewport area in absolute world units.
		 * @param force Use the given world units despite their illegal values (like negative coordinates etc.).
		 */
		void setWorldY(int y, bool force=false);

		/**
		 * Set the world width of the viewport.
		 * WARNING! Repaint is not done automatically!
		 * 
		 * @param w Width of the new viewport area in absolute world units.
		 * @param force Use the given world units despite their illegal values (like negative coordinates etc.).
		 */
		void setWorldWidth(int w, bool force=false) ;

		/**
		 * Set the world height of the viewport.
		 * WARNING! Repaint is not done automatically!
		 * 
		 * @param h Height of the new viewport area in absolute world units.
		 * @param force Use the given world units despite their illegal values (like negative coordinates etc.).
		 */
		void setWorldHeight(int h, bool force=false);
		
		/**
		 * Return the X coordinate of top-left point of the viewport.
		 * 
		 * @return X coordinate of the top-left point of the viewport in absolute world units.
		 */
		const int worldX() { return _worldX; }

		/**
		 * Return the Y coordinate of top-left point of the viewport.
		 * 
		 * @return Y coordinate of the top-left point of the viewport in absolute world units.
		 */
		const int worldY() { return _worldY; }

		/**
		 * Return the width of the viewport.
		 * 
		 * @return Width of the drawable viewport in absolute world units.
		 */
		const int worldWidth() { return _worldW; }

		/**
		 * Return the height of the viewport.
		 * 
		 * @return Height of the drawable viewport in absolute world units.
		 */
		const int worldHeight() { return _worldH; }

		/**
		 * Return the zoom level of the viewport.
		 * 
		 * @return Zoom level of the viewport. (1.0 = 100%, 1.5 = 150% etc.)
		 */
		const float zoom() { return _zoom; }

		/**
		 * Set the world coordinates of the viewport.
		 * WARNING! Repaint is not done automatically!
		 * 
		 * @param x Top-left X coordinate of the new viewport area in absolute world units.
		 * @param y Top-left Y coordinate of the new viewport area in absolute world units.
		 * @param w Width of the new viewport area in absolute world units.
		 * @param h Height of the new viewport area in absolute world units.
		 * @param force Use the given world units despite their illegal values (like negative coordinates etc.).
		 */ 
		void setWorldCoords(int x, int y, int w, int h, bool force=false);
		
		/**
		 * This is an overloaded member function, provided for convenience.
		 * 
		 * Set the world coordinates of the viewport.
		 * WARNING! Repaint is not done automatically!
		 * 
		 * @param r QRect of the new viewport area in absolute world units.
		 * @param force Use the given world units despite their illegal values (like negative coordinates etc.).
		 */ 
		void setWorldCoords(QRect *r, bool force=false) { setWorldCoords(r->x(),r->y(),r->width(),r->height(), force); }
		
		/**
		 * This is an overloaded member function, provided for convenience.
		 *
		 * Set the world coordinates of the viewport. Width and Height are not changed.
		 * WARNING! Repaint is not done automatically!
		 * 
		 * @param x Top-left X coordinate of the new viewport area in absolute world units.
		 * @param y Top-left Y coordinate of the new viewport area in absolute world units.
		 * @param force Use the given world units despite their illegal values (like negative coordinates etc.).
		 */
		void setWorldCoords(int x, int y, bool force=false);
		
		/**
		 * Set the world coordinates of the viewport, so the given coordinates are the center of the new viewport area.
		 * If the area has for eg. negative top-left coordinates, the area is moved to the (0,0) coordinates if force is unset.
		 * Width and Height are not changed.
		 * WARNING! Repaint is not done automatically!
		 * 
		 * @param x Center X coordinate of the new viewport area in absolute world units.
		 * @param y Center Y coordinate of the new viewport area in absolute world units.
		 * @param force Use the given world units despite their illegal values (like negative coordinates etc.).
		 */
		void setCenterCoords(int x, int y, bool force=false);
		
		/**
		 * Zoom to the given level to given direction.
		 * WARNING! Repaint is not done automatically!
		 * 
		 * @param z Zoom level. (1.0 = 100%, 1.5 = 150% etc.)
		 * @param x X coordinate of the point of the zoom direction. 
		 * @param y Y coordinate of the point of the zoom direction.
		 * @param force Use the given world units despite their illegal values (like negative coordinates etc.).
		 */
		void setZoom(float z, int x=0, int y=0, bool force = false);
		
		/**
		 * This is an overloaded member function, provided for convenience.
		 * 
		 * Zoom to the given level to given direction.
		 * WARNING! Repaint is not done automatically!
		 * 
		 * @param z Zoom level. (1.0 = 100%, 1.5 = 150% etc.)
		 * @param p QPoint of the zoom direction.
		 * @param force Use the given world units despite their illegal values (like negative coordinates etc.).
		 */
		void setZoom(float z, QPoint p, bool force = false) { setZoom(z, p.x(), p.y(), force); }		
		
		/**
		 * Draw the border with the given pen style, color, width and other pen settings.
		 * Enable border.
		 * 
		 * @param pen QPen object used for drawing the ViewPort border.
		 */
		void setBorder(const QPen pen);
		
		/**
		 * Fill the background with the given brush style and color.
		 * 
		 * @param pen QBrush object used for drawing the ViewPort background.
		 */
		void setBackground(const QBrush brush);
		
		/**
		 * Disable border.
		 */
		void unsetBorder();
		
	signals:
		/**
		 * Default mouse press event signal automatically emmitted.
		 * 
		 * @param e Mouse event which gets processed.
		 */
		void mousePressEvent(QMouseEvent *e);
		
		/**
		 * Default wheel event signal automatically emmitted.
		 * 
		 * @param e Wheel event which gets processed.
		 */		
		void wheelEvent(QWheelEvent *);
		
		/**
		 * This signal is emitted when mousePressEvent() is called. Parent class is usually connected to this event.
		 * It adds another argument to the mousePressEvent() function - pointer to this viewport.
		 * This is useful when a parent class wants to know which class the signal was emmitted by.
		 * 
		 * @param e Mouse event which gets processed.
		 * @param p Coordinates of the mouse cursor in absolute world values.
		 * @param v Pointer to this viewport (the viewport which emmitted the signal).
		 */
		void CAMousePressEvent(QMouseEvent *e, QPoint p, CAViewPort *v);

		/**
		 * This signal is emitted when wheelEvent() is called. Parent class is usually connected to this event.
		 * It adds another argument to the wheelEvent() function - pointer to this viewport.
		 * This is useful when a parent class wants to know which class the signal was emmitted by.
		 * 
		 * @param e Wheel event which gets processed.
		 * @param p Coordinates of the mouse cursor in absolute world values.
		 * @param v Pointer to this viewport (the viewport which emmitted the signal).
		 */
		void CAWheelEvent(QWheelEvent *e, QPoint p, CAViewPort *v);
		
	private slots:
		/**
		 * Process the mousePressEvent() signal.
		 * A new signal is emitted: CAMouseEvent(), which usually gets processed by the parent class.
		 * 
		 * @param e Mouse event which gets forwarded.
		 */
		void processMousePressEvent(QMouseEvent *e);

		/**
		 * Process the wheelEvent() signal.
		 * A new signal is emitted: CAWheelEvent(), which usually gets processed by the parent class.
		 * 
		 * @param e Wheel event which gets forwarded.
		 */
		void processWheelEvent(QWheelEvent *e);
		
		/**
		 * Process the Horizontal scroll bar event.
		 * This method is called when the horizontal scrollbar changes its value, let it be manually or due to user interaction.
		 * 
		 * @param val Value the scrollbar changed its value to.
		 */
		void processHScrollBarEvent(int val);

		/**
		 * Process the Vertical scroll bar event.
		 * This method is called when the vertical scrollbar changes its value, let it be manually or due to user interaction.
		 * 
		 * @param val Value the scrollbar changed its value to.
		 */
		void processVScrollBarEvent(int val);
		
		/**
		 * Called when user resizes the form.
		 * Note that repaint() event is also triggered when the internal drawable canvas changes its size (for eg. when scrollbars are shown/hidden) and the size of CAViewPort does not change.
		 * 
		 * @param e QResizeEvent which gets processed.
		 */
		void resizeEvent(QResizeEvent *e);
		
		/**
		 * General paint event.
		 * All the music elements get rendered in this method.
		 * 
		 * @param p QPaintEvent which gets processed.
		 */
		void paintEvent(QPaintEvent *p);
		
		/**
		 * Is the given element selected.
		 * 
		 * @param elt Pointer to the element which we query.
		 * @return True, if the given element is selected, false otherwise.
		 */
		bool isSelected(CADrawableMusElement *elt) { return (_musElementSelection.contains(elt)); }
		
	private:
		////////////////////////////////////////////////
		//General properties
		////////////////////////////////////////////////
		CAKDTree _drawableMList;	///The list of music elements stored in a tree for faster lookup and other operations. Every viewport has its own list of drawable elements and drawable objects themselves!
		CAKDTree _drawableCList;	///The list of context drawable elements (staffs, lyrics etc.). Every viewport has its own list of drawable elements and drawable objects themselves!
		CASheet *_sheet;	///Pointer to the CASheet which the viewport represents
		
		QList<CADrawableMusElement *> _musElementSelection;	///The set of elements being selected.
		CADrawableContext *_currentContext;	///The pointer to the currently active context (staff, lyrics).
		
		int _worldX, _worldY, _worldW, _worldH;	///Absolute world coordinates of the area the viewport is currently showing.
		float _zoom;	///Zoom level of the viewport (1.0 = 100%, 1.5 = 150% etc.).
		
		////////////////////////////////////////////////
		//Widgets and appearance
		////////////////////////////////////////////////
		QGridLayout *_layout;	///Grid layout for placing the scrollbars at the right and the bottom.
		QWidget *_canvas;	///Virtual canvas which represents the size of the drawable area. All its signals are forwarded to CAViewPort.
		QScrollBar *_hScrollBar, *_vScrollBar;	///Horizontal/vertical scrollbars
		bool _drawBorder;	///Should the border be drawn or not
		QPen _borderPen;	///Pen which the border is drawn by
		QBrush _backgroundBrush;	///Brush which the background is drawn by

		////////////////////////////////////////////////
		//Widgets behaviour
		////////////////////////////////////////////////
		char _scrollBarsVisible; ///Are the scrollbars always visible/never/if needed. Use CAViewPort::ScrollBarAlwaysVisible, CAViewPort::ScrollBarAlwaysHidden or CAViewPort::ScrollBarShowIfNeeded.
		bool _allowManualScroll; ///Does the scrollbars actually react on user actions - sometimes we only want the scrollbars to show the current location of the score and don't do anything

		////////////////////////////////////////////////
		//Internal properties
		////////////////////////////////////////////////
		int _oldWorldX, _oldWorldY, _oldWorldW, _oldWorldH;	///Old coordinates used before the repaint. This is needed so only the new part of the viewport gets repainted when panning.
		bool _holdRepaint;	///Dirty flag to prevent multiple repaintings.
		bool _checkScrollBarsDeadLock;	///Dirty flag to prevent recursive checkScrollBars() calls.
		bool _hScrollBarDeadLock;	///Dirty flag to prevent recursive scrollbar calls when its value is manually changed.
		bool _vScrollBarDeadLock;	///Dirty flag to prevent recursive scrollbar calls when its value is manually changed.
};

#endif /*SCOREVIEWPORT_H_*/
