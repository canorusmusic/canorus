/*!
	Copyright (c) 2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "widgets/tabwidget.h"
#include <QMouseEvent>
#include <QTabBar>

/*!
	\class CATabWidget
	\brief Tab widget for the score view inside the main window

	This class is usually used to store multiple sheets inside the main window.
	For now it offers double-click new sheet shortcut and movable tabs.
*/

CATabWidget::CATabWidget( QWidget *parent )
 : QTabWidget(parent) {
	setMovable(false);
	connect( tabBar(), SIGNAL(tabMoved(int, int)), this, SIGNAL(CAMoveTab(int,int)) );
}

CATabWidget::~CATabWidget() {
}

/*!
	Processes the double click on empty space for the new tab action.
	The event needs to be implemented in the TabBar and not TabWidget to effect
	the double clicks inside the tab area only.
*/
void CATabWidget::mouseDoubleClickEvent( QMouseEvent * event ) {
	if (event->y()>tabBar()->y() && tabBar()->tabAt(event->pos())==-1) {
		emit CANewTab();
	}
}

/*!
	Disable moving sheets, if only one present.
*/
void CATabWidget::tabInserted(int) {
	setMovable(count()>=2);
}

/*!
	Disable moving sheets, if only one present.
*/
void CATabWidget::tabRemoved(int) {
	setMovable(count()>=2);
}
