/*!
	Copyright (c) 2006-2007, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QApplication>
#include <QDesktopWidget>
#include <QWheelEvent>
#include <QMainWindow>
#include <QToolBar>
#include <QWidgetAction>

#include "widgets/menutoolbutton.h"
#include "ui/mainwin.h"

/*!
	\class CAMenuToolButton
	\brief Tool button with a menu at the side and a button box when clicked on
	
	This widget looks like a button with a small dropdown arrow at the side which opens a
	button group box of various elements. User can add buttons by calling
	addButton(QIcon icon, int Id). When the element is selected, the action's icon is
	switched to the selected element's and a signal toggled(bool checked, int id) is emitted.
	
	The class primarily consists of 3 elements:
		- the base class QToolButton with enabled side menu
		- QButtonGroup
			The backend list of buttons and their Ids (QGroupBox doesn't support button Ids)
		- QGroupBox
			The widget that is shown when menu arrow is clicked.	
*/ 

/*!
	Constructs the button menu with the given \a title and \a parent.
*/
CAMenuToolButton::CAMenuToolButton( QString title, int numIconsRow, QWidget * parent) 
 : CAToolButton( parent ) {
	setSpacing( 4 );
	setLayoutMargin( 5 );
	setMargin( 0 );
	setCheckable( true );
	setNumIconsPerRow( numIconsRow );
	
	// Size policy: Expanding / Expanding
    QSizePolicy boxSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    boxSizePolicy.setHorizontalStretch( 0 );
    boxSizePolicy.setVerticalStretch( 0 );
    QSizePolicy widgetSizePolicy = sizePolicy();
    boxSizePolicy.setHeightForWidth( widgetSizePolicy.hasHeightForWidth() );
    
    // Visual group box for the button menu
	_groupBox = new QGroupBox( title, 0 );
	_groupBox->setWindowFlags( Qt::FramelessWindowHint );
	_groupBox->hide();
    boxSizePolicy.setHeightForWidth( _groupBox->sizePolicy().hasHeightForWidth() );
    _groupBox->setSizePolicy( boxSizePolicy );
	_groupBox->setBackgroundRole( QPalette::Button );
	_groupBox->setAutoFillBackground( true );
    
	// Layout for visual group box
	_boxLayout = new QGridLayout( _groupBox );
	_boxLayout->setSpacing( spacing() );
    _boxLayout->setMargin( layoutMargin() );
    setSizePolicy( boxSizePolicy );
    
	// Abstract group for mutual exclusive toggle
	_buttonGroup = new QButtonGroup( _groupBox );
	
	connect( _buttonGroup, SIGNAL(buttonPressed( int )), 
	         this, SLOT( hideButtons( int ) ) );
	
	QToolButton::setDefaultAction( 0 );
    
	// Actual positions of the buttons in the button menu layout
	_buttonXPos = _buttonYPos = 0;
}

/*!
	Destructs the button menu.
*/
CAMenuToolButton::~CAMenuToolButton() {
	for (int i = 0; i < _buttonList.size(); ++i)
		delete _buttonList[i];
	delete _buttonGroup;
	delete _groupBox;
}



/*!
	Adds a Tool button to the menu with the given \a icon and \a buttonId.
*/
void CAMenuToolButton::addButton( const QIcon icon, int buttonId, const QString toolTip ) {
	QToolButton *button;
	QFontMetrics metrics ( _groupBox->font() );
	int iconSize = 24,          // Size of Icon
	    xMargin  = _margin * 2, // Margin around the buttons
	    yMargin  = _margin * 2 + metrics.height(), // includes the height of the menu text
	    xSize    = iconSize,
	    ySize    = iconSize;
	
	// Create new button for menu
	button = new QToolButton( _groupBox );
	button->setIcon( icon );
	button->setIconSize( QSize(iconSize, iconSize) );
	button->setCheckable( true );
	button->setToolTip(toolTip);
	// Useful if you want to switch icons of an associated toolbar
	button->setObjectName( objectName() );
	_buttonList << button;
	
	// Add it to the abstract group
    _buttonGroup->addButton( _buttonList.last(), buttonId );
    
    // Create menu that has miNumIconsRow buttons in each row
    if( _buttonXPos >= numIconsPerRow() )
    {
    	_buttonXPos = 0;
    	_buttonYPos++;
    }
    
    // Add it to the button menu layout
    _boxLayout->addWidget( button, _buttonYPos, _buttonXPos, Qt::AlignLeft );
    _buttonXPos++;
    if( _buttonYPos > 0 ) {
	    xSize = numIconsPerRow() * (_spacing+button->width()/3);
    } else {
    	xSize = _buttonXPos * (_spacing+button->width()/3);
    }
	ySize = (_buttonYPos+1) * (_spacing+button->height());
	_groupBox->setMinimumSize( xMargin + xSize, yMargin + ySize );
}

/*!
	Shows the button menu (connected to aboutToShow signal).
	
	\warning Coordinates of groupBox are calculated according to its parent (main window) and not this parent (QToolBar).
*/	
void CAMenuToolButton::showButtons() {
	QToolBar *toolBar = dynamic_cast<QToolBar*>(parent());
	int x=0, y=0;
	_groupBox->hide(); // hide/reshow if the button box widget is behind the main window
	_groupBox->show();
	
	if ( mainWin() && toolBar ) {
		QPoint topLeft = mapToGlobal(QPoint(0,0)); // get the absolute coordinates of top-left corner of the button
		
		// Set buttons box coordinates which fit on the main window
		if (mainWin()->toolBarArea(toolBar) == Qt::LeftToolBarArea) {
			if (topLeft.x() + width() + _groupBox->width() > mainWin()->width()) x = mainWin()->width() - _groupBox->width();
			else x = topLeft.x() + width();
			
			if (topLeft.y() + _groupBox->height() > mainWin()->height()) y = mainWin()->height() - _groupBox->height();
			else y = topLeft.y();
		} else
		if (mainWin()->toolBarArea(toolBar) == Qt::TopToolBarArea) {
			if (topLeft.x() + _groupBox->width() > mainWin()->width()) x = mainWin()->width() - _groupBox->width();
			else x = topLeft.x();
			
			if (topLeft.y() + height() + _groupBox->height() > mainWin()->height()) y = mainWin()->height() - _groupBox->height();
			else y = topLeft.y() + height();
		} else
		if (mainWin()->toolBarArea(toolBar) == Qt::RightToolBarArea) {
			if (topLeft.x() - width() - _groupBox->width() < 0) x = 0;
			else x = topLeft.x() - _groupBox->width();
			
			if (topLeft.y() + _groupBox->height() > mainWin()->height()) y = mainWin()->height() - _groupBox->height();
			else y = topLeft.y();
		} else
		if (mainWin()->toolBarArea(toolBar) == Qt::BottomToolBarArea) {
			if (topLeft.x() + _groupBox->width() > mainWin()->width()) x = mainWin()->width() - _groupBox->width();
			else x = topLeft.x();
			
			if (topLeft.y() - _groupBox->height() < 0) y = 0;
			else y = topLeft.y() - _groupBox->height();
		}
	}
	
	_groupBox->move(x,y);
}

/*!
	Hides the buttons menu, changes the current id and emits the toggled() signal.
*/
void CAMenuToolButton::hideButtons( int id ) {
	if ( _buttonGroup->button(id) ) {
		setCurrentId( id );
		if (isChecked())
			emit toggled( true, id ); // if already on and in any button group
		else
			click(); // trigger any button groups
		setChecked(true); // turn it on if it can turn off
	}
	hideButtons();
}

/*!
	Hides the buttons menu only.
*/
void CAMenuToolButton::hideButtons() {
	_groupBox->hide();
}

/*!
	Cycle through properties using the mouse wheel.
*/
void CAMenuToolButton::wheelEvent( QWheelEvent *event ) {
	QList<QAbstractButton*> buttonList = _buttonGroup->buttons();
	QAbstractButton *button = _buttonGroup->button(currentId());
	int newIdx = buttonList.indexOf(button) + (event->delta()>0?-1:1);
	
	if (newIdx == buttonList.size())
		newIdx = 0;
	else if (newIdx == -1)
		newIdx = buttonList.size()-1;
	
	setCurrentId( _buttonGroup->id(buttonList[newIdx]) );
	if (isChecked())
		emit toggled( true, currentId() ); // if already on and in any button group
	else
		click(); // trigger any button groups
	setChecked(true); // turn it on if it can turn off
}

/*!
	Sets the currently selected item by passing the item index.
	The current icon of the button is changed to the item ones.
	The current tool tip is also changed.
	
	Does not change the current item, if the item is not part of the button box.
*/
void CAMenuToolButton::setCurrentId(int id) {
	if (_buttonGroup->button( currentId() ))
		_buttonGroup->button( currentId() )->setChecked(false);
	
	CAToolButton::setCurrentId(id);
	
	if ( !_buttonGroup->button(id) )
		return;
	
	_buttonGroup->button(id)->setChecked(true);
	if ( defaultAction() )
		defaultAction()->setIcon( _buttonGroup->button(id)->icon() );
}

/*!
	\fn void CAButtonMenu::toggled( bool checked, int id )
	
	Signal sent when the button is clicked or an element is selected and changed.
*/
