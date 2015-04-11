/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "widgets/view.h"
#include "canorus.h"

const int CAView::DEFAULT_VIEW_WIDTH = 600;
const int CAView::DEFAULT_VIEW_HEIGHT = 400;

/*!
	\class CAView
	\brief Base class for various views

	CAView is the base widget for different types of views of the document.
	Viewport can represent a score view, envelope view, score source view etc.
*/

/*!
	The default constructor.
	Creates a widget with the parent widget \a p.
*/
CAView::CAView(QWidget *p)
 : QWidget(p) {
 	setGeometry( 0, 0, 0, 0 );
}

/*!
	Destructor.
*/
CAView::~CAView() {
	CACanorus::removeView(this);
}

void CAView::mousePressEvent(QMouseEvent *e) {
	QWidget::mousePressEvent(e);
	emit clicked();
}

/*!
	\fn CAView *CAView::clone()

	Returns a pointer to the new cloned view with the same parent.
	This function is usually called when a user creates a new docked view.
*/

/*!
	\fn CAView *CAView::clone(QWidget *parent)

	Returns a pointer to the new cloned view with the parent \a parent.
	This function is usually called when a user creates a new undocked view.
*/

/*!
	\fn void CAView::rebuild()

	Synchronizes/Rebuilds the UI part from the abstract one.
	This function is usually called when a user creates changes to the score or a new view is introduced.
*/

/*!
	\fn void CAView::clicked

	This signal is emitted when mousePressEvent() is called. Parent class is usually connected to this event.
*/

/*!
	\enum CAView::CAViewType
	Holds different view types:
		- ScoreView - The main view of the score. All the music elements (staffs, notes, rests) are rendered to this view.
		- SourceView - Score source view in various syntices (LilyPond, CanorusML etc.).
*/
