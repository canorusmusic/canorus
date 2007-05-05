/*! 
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "widgets/viewport.h"
#include "core/canorus.h"

/*!
	\class CAViewPort
	\brief Base class for various viewports
	
	CAViewPort is the base widget for different types of views of the document.
	Viewport can represent a score view, envelope view, score source view etc.
*/

/*!
	The default constructor.
	Creates a widget with the parent widget \a p.
*/
CAViewPort::CAViewPort(QWidget *p) : QWidget(p) {
	_parent = p;
}

/*!
	Destructor.
*/
CAViewPort::~CAViewPort() {
	CACanorus::removeViewPort(this);
}

void CAViewPort::mousePressEvent(QMouseEvent *e) {
	QWidget::mousePressEvent(e);
	emit CAMousePressEvent( e, this );
}

void CAViewPort::mouseMoveEvent(QMouseEvent *e) {
	QWidget::mouseMoveEvent(e);
	emit CAMouseMoveEvent( e, this );
}

void CAViewPort::mouseReleaseEvent(QMouseEvent *e) {
	QWidget::mouseReleaseEvent(e);
	emit CAMouseReleaseEvent( e, this );
}

void CAViewPort::wheelEvent(QWheelEvent *e) {
	QWidget::wheelEvent(e);
	emit CAWheelEvent( e, this );
}
		
void CAViewPort::keyPressEvent(QKeyEvent *e) {
	QWidget::keyPressEvent(e);
	emit CAKeyPressEvent( e, this );
}

/*!
	\fn CAViewPort *CAViewPort::clone()
	
	Returns a pointer to the new cloned viewport with the same parent.
	This function is usually called when a user creates a new docked view.
*/

/*!
	\fn CAViewPort *CAViewPort::clone(QWidget *parent)
	
	Returns a pointer to the new cloned viewport with the parent \a parent.
	This function is usually called when a user creates a new undocked view.
*/

/*!
	\fn void CAViewPort::rebuild()
	
	Synchronizes/Rebuilds the UI part from the abstract one.
	This function is usually called when a user creates changes to the score or a new viewport is introduced.
*/

/*!
	\fn void CAViewPort::CAMousePressEvent(QMouseEvent *e, CAViewPort *v)
	
	This signal is emitted when mousePressEvent() is called. Parent class is usually connected to this event.
	It adds another argument to the mousePressEvent() function - pointer to this viewport.
	This is useful when a parent class wants to know which class the signal was emmitted by.
	
	\param e Mouse event which gets processed.
	\param v Pointer to this viewport (the viewport which emmitted the signal).
*/

/*!
	\fn void CAViewPort::CAMouseMoveEvent(QMouseEvent *e, CAViewPort *v)
	
	This signal is emitted when mouseMoveEvent() is called. Parent class is usually connected to this event.
	It adds another argument to the mouseMoveEvent() function - pointer to this viewport.
	This is useful when a parent class wants to know which class the signal was emmitted by.
	
	\param e Mouse event which gets processed.
	\param v Pointer to this viewport (the viewport which emmitted the signal).
*/

/*!
	\fn void CAViewPort::CAMouseReleaseEvent(QMouseEvent *e, CAViewPort *v)
	
	This signal is emitted when mouseReleaseEvent() is called. Parent class is usually connected to this event.
	It adds another argument to the mouseReleaseEvent() function - pointer to this viewport.
	This is useful when a parent class wants to know which class the signal was emmitted by.
	
	\param e Mouse event which gets processed.
	\param v Pointer to this viewport (the viewport which emmitted the signal).
*/

/*!
	\fn void CAViewPort::CAWheelEvent(QWheelEvent *e, CAViewPort *v)
	
	This signal is emitted when wheelEvent() is called. Parent class is usually connected to this event.
	It adds another argument to the wheelEvent() function - pointer to this viewport.
	This is useful when a parent class wants to know which class the signal was emmitted by.
	
	\param e Wheel event which gets processed.
	\param v Pointer to this viewport (the viewport which emmitted the signal).
*/

/*!
	\fn void CAKeyPressEvent(QKeyEvent *e, CAViewPort *v)

	This signal is emitted when keyPressEvent() is called. Parent class is usually connected to this event.
	It adds another argument to the keyPressEvent() function - pointer to this viewport.
	This is useful when a parent class wants to know which class the signal was emmitted by.
	
	\param e Key event which gets processed.
	\param v Pointer to this viewport (the viewport which emmitted the signal).
*/

/*!
	\enum CAViewPort::CAViewPortType
	Holds different viewport types:
		- ScoreViewPort - The main view of the score. All the music elements (staffs, notes, rests) are rendered to this viewport.
		- SourceViewPort - Score source view in various syntices (LilyPond, CanorusML etc.).
*/
