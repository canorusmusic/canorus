/** @file widgets/viewport.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QWidget>

#define DEFAULT_VIEWPORT_WIDTH 400
#define DEFAULT_VIEWPORT_HEIGHT 400

/**
 * CAViewPort is the base widget for different types of views of the document.
 * Viewport can represent a score view, envelope view, score source view etc.
 */
class CAViewPort : public QWidget {
	Q_OBJECT
	
	public:
		/**
		 * The default constructor.
		 * 
		 * @param parent The parent widget.
		 */
		CAViewPort(QWidget *parent=0);
		
		/**
		 * Destructor.
		 */
		virtual ~CAViewPort();
		
		/**
		 * enum CAViewPortType holds different viewport types:
		 * - ScoreViewPort - The main view of the score. All the music elements (staffs, notes, rests) are rendered to this viewport. 
		 */
		enum CAViewPortType {
			ScoreViewPort,
			SourceViewPort
		};
		
		/**
		 * Return the viewport type. See enum CAViewPortType for viewport types.
		 * 
		 * @return Return the viewport's type in CAViewPortType type.
		 */
		CAViewPortType viewPortType() { return _viewPortType; }
		
		QWidget *parent() { return _parent; }
		
		/**
		 * Clone the viewport.
		 *
		 * @return Pointer to the new viewPort class.
		 */
		virtual CAViewPort *clone() = 0;

		/**
		 * This is an overloaded member function, provided for convenience.
		 * 
		 * Clone the viewport with a different parent widget.
		 *
		 * @param parent New parent of the cloned widget.
		 * @return Pointer to the new viewPort class.
		 */
		virtual CAViewPort *clone(QWidget *parent) = 0;
		
		/**
		 * Synchronize/Rebuild the UI part with the logical one.
		 */
		virtual void rebuild() = 0;
		
	signals:
		/**
		 * Default mouse press event signal automatically emmitted.
		 * 
		 * @param e Mouse event which gets processed.
		 */
		void CAMousePressEvent(QMouseEvent *e);
		
		/**
		 * Default wheel event signal automatically emmitted.
		 * 
		 * @param e Wheel event which gets processed.
		 */
		void CAWheelEvent(QWheelEvent *);
		
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
		 * This signal is emitted when mouseMoveEvent() is called. Parent class is usually connected to this event.
		 * It adds another argument to the mouseMoveEvent() function - pointer to this viewport.
		 * This is useful when a parent class wants to know which class the signal was emmitted by.
		 * 
		 * @param e Mouse event which gets processed.
		 * @param p Coordinates of the mouse cursor in absolute world values.
		 * @param v Pointer to this viewport (the viewport which emmitted the signal).
		 */
		void CAMouseMoveEvent(QMouseEvent *e, QPoint p, CAViewPort *v);

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
		
		/**
		 * This signal is emitted when keyPressEvent() is called. Parent class is usually connected to this event.
		 * It adds another argument to the keyPressEvent() function - pointer to this viewport.
		 * This is useful when a parent class wants to know which class the signal was emmitted by.
		 * 
		 * @param e Key event which gets processed.
		 * @param v Pointer to this viewport (the viewport which emmitted the signal).
		 */
		void CAKeyPressEvent(QKeyEvent *e, CAViewPort *v);
	
	protected:
		////////////////////////////////////////////////
		//General properties
		////////////////////////////////////////////////
		CAViewPortType _viewPortType;

		////////////////////////////////////////////////
		//Widgets
		////////////////////////////////////////////////
		QWidget *_parent;	///Pointer to the parent widget.
		
};
#endif
