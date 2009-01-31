/*! 
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "widgets/viewport.h"
#include "canorus.h"

const int CAViewPort::DEFAULT_VIEWPORT_WIDTH = 600;
const int CAViewPort::DEFAULT_VIEWPORT_HEIGHT = 400;

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
CAViewPort::CAViewPort(QWidget *p)
 : QWidget(p) {
 	setGeometry( 0, 0, 0, 0 );
}

/*!
	Destructor.
*/
CAViewPort::~CAViewPort() {
	CACanorus::removeViewPort(this);
}

void CAViewPort::mousePressEvent(QMouseEvent *e) {
	QWidget::mousePressEvent(e);
	emit clicked();
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
	\fn void CAViewPort::clicked
	
	This signal is emitted when mousePressEvent() is called. Parent class is usually connected to this event.
*/

/*!
	\enum CAViewPort::CAViewPortType
	Holds different viewport types:
		- ScoreViewPort - The main view of the score. All the music elements (staffs, notes, rests) are rendered to this viewport.
		- SourceViewPort - Score source view in various syntices (LilyPond, CanorusML etc.).
*/
