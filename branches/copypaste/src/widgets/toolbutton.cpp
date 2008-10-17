/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "widgets/toolbutton.h"
#include "ui/mainwin.h"

#include <QMainWindow>
#include <QMouseEvent>
#include <QStyle>
#include <QStyleOptionToolButton>

/*!
	\class CAToolButton
	\brief Tool button with a menu at the side
	
	This widget looks like a button with a small dropdown arrow at the side which opens a
	custom widget (also called "buttons") where user chooses from various actions then.
	When the element is selected, the action's icon is replaced with the previous icon on
	the toolbutton and a signal toggled(bool checked, int id) is emitted.
	
	\remarks Subclasses must call setPopupWidget() on the floating widget once it's initialized. Showing/hiding is handled in CAToolButton. Subclasses can connect to the show() or hide() signals, which are emitted just before showing or hiding the widget.
*/ 

CAToolButton::CAToolButton( QWidget *parent )
 : QToolButton( parent ) {
	setMainWin( parent?dynamic_cast<CAMainWin*>(parent):0 );
	setPopupMode( QToolButton::MenuButtonPopup );	
	_popupWidget = new CAToolButtonPopup();
}

CAToolButton::~CAToolButton() {
	delete _popupWidget;
}

/*!
	Shows the popup widget if it's set
*/
void CAToolButton::showButtons() {
	if(_popupWidget) { 
		_popupWidget->move( calculateTopLeft( _popupWidget->sizeHint() ) );
		_popupWidget->show();
	}
}

/*!
	Hides the popup widget if it's set
*/
void CAToolButton::hideButtons() {
	if (_popupWidget) { 
		_popupWidget->hide(); 
	} 
}
/*!
	This function is overriden here in order to show buttons when clicked on the arrow.
*/
void CAToolButton::mousePressEvent( QMouseEvent *e ) {
	QStyleOptionToolButton opt;
    opt.init(this);	
	opt.subControls |= QStyle::SC_ToolButtonMenu;
	opt.features |= QStyleOptionToolButton::Menu;
    
	QRect popupr = style()->subControlRect(QStyle::CC_ToolButton, &opt,
	                                       QStyle::SC_ToolButtonMenu, this);
	if (popupr.isValid() && popupr.contains(e->pos())) {
		if ( !buttonsVisible() ) {
			showButtons();
		} else {
			hideButtons();
		}
	}
	QToolButton::mousePressEvent(e);
}

/*!
	Emits toggled( bool, int ) signal.
	
	\sa handleToggled()
*/
void CAToolButton::handleTriggered() {
	if (!defaultAction()->isCheckable())
		emit toggled( false, currentId() );
}

/*!
	Emits toggled( bool, int ) signal.
	
	\sa handleTriggered()
*/
void CAToolButton::handleToggled( bool checked ) {
	emit toggled( checked, currentId() );
}

/*!
	Sets the new default action \a a and connects some signals to custom slots made
	by CAMenuToolButton. Also calls QToolButton::setDefaultAction().
*/
void CAToolButton::setDefaultAction( QAction *a ) {
	if ( defaultAction() ) {
		disconnect( defaultAction(), SIGNAL(toggled(bool)), this, SLOT(handleToggled(bool)) );
		disconnect( defaultAction(), SIGNAL(triggered()), this, SLOT(handleTriggered()) );
	}
	
	connect( a, SIGNAL(toggled(bool)), this, SLOT(handleToggled(bool)) );
	connect( a, SIGNAL(triggered()), this, SLOT(handleTriggered()) );
	a->setCheckable( true );
	QToolButton::setDefaultAction( a );
}

/*!
	This function returns the absolute top-left coordinate where the popup menu or whichever
	widget should appear when the users clicks on the button.
	
	The problem is that the popup widget should be completely visible in whichever part of
	the screen the toolbutton is located. Popup widget should also always stick to one corner
	of the toolbutton.
	
	Parameter \a size is the width and height needed for the whole widget to appear.
*/
QPoint CAToolButton::calculateTopLeft( QSize size ) {
	int x=0, y=0;
	QToolBar *toolBar = dynamic_cast<QToolBar*>(parent());
	if ( mainWin() && toolBar ) {
		QPoint topLeft = mapToGlobal(QPoint(0,0)); // get the absolute coordinates of top-left corner of the button
		
		// Set buttons box coordinates which fit on the main window
		if (mainWin()->toolBarArea(toolBar) == Qt::LeftToolBarArea) {
			if (topLeft.x() + width() + size.width() > mainWin()->width()) x = mainWin()->width() - size.width();
			else x = topLeft.x() + width();
			
			if (topLeft.y() + size.height() > mainWin()->height()) y = mainWin()->height() - size.height();
			else y = topLeft.y();
		} else
		if (mainWin()->toolBarArea(toolBar) == Qt::TopToolBarArea) {
			if (topLeft.x() + size.width() > mainWin()->width()) x = mainWin()->width() - size.width();
			else x = topLeft.x();
			
			if (topLeft.y() + height() + size.height() > mainWin()->height()) y = mainWin()->height() - size.height();
			else y = topLeft.y() + height();
		} else
		if (mainWin()->toolBarArea(toolBar) == Qt::RightToolBarArea) {
			if (topLeft.x() - width() - size.width() < 0) x = 0;
			else x = topLeft.x() - size.width();
			
			if (topLeft.y() + size.height() > mainWin()->height()) y = mainWin()->height() - size.height();
			else y = topLeft.y();
		} else
		if (mainWin()->toolBarArea(toolBar) == Qt::BottomToolBarArea) {
			if (topLeft.x() + size.width() > mainWin()->width()) x = mainWin()->width() - size.width();
			else x = topLeft.x();
			
			if (topLeft.y() - size.height() < 0) y = 0;
			else y = topLeft.y() - size.height();
		}
	}
	
	return QPoint(x,y);
}
