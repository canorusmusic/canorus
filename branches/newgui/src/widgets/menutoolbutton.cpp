/*! \file widgets/buttonmenu.cpp
	
	Copyright (c) 2006-2007, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QApplication>
#include <QDesktopWidget>

#include "widgets/menutoolbutton.h"

/*!
	\class CAMenuToolButton
	\brief Tool button with a menu at the side
	
	This widget looks like a button with a small dropdown arrow at the side which opens a
	button group box of various elements. User can add buttons by calling
	addButton(QIcon icon, int Id). When the element is selected, the action's icon is
	switched to the selected element's and a signal toggled(bool checked, int id) is emitted.
	
	The class primarily consists of 4 elements:
		- the base class QToolButton with enabled side menu
		- QMenu
			Needed for QToolButton::setMenu() and to catch aboutToShow() signal.
		- QButtonGroup
			The backend list of buttons and their Ids (QGroupBox doesn't support button Ids)
		- QGroupBox
			The widget that is shown when menu arrow is clicked.	
*/ 

/*!
	Constructs the button menu with the given \a title and \a parent.
*/
CAMenuToolButton::CAMenuToolButton( QString title, int numIconsRow, QWidget * parent) 
 : QToolButton( parent ) {
	setSpacing( 4 );
	setLayoutMargin( 5 );
	setMargin( 0 );
	setCheckable( true );
	setNumIconsPerRow( numIconsRow );
	
	// Size policy: Expanding / Expanding
    QSizePolicy boxSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    boxSizePolicy.setHorizontalStretch( 0 );
    boxSizePolicy.setVerticalStretch( 0 );
    QSizePolicy widgetSizePolicy = sizePolicy();
    boxSizePolicy.setHeightForWidth( widgetSizePolicy.hasHeightForWidth() );
    
    // Visual group box for the button menu
	_groupBox = new QGroupBox( title, parent );
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
	
	setMenu( _menu = new QMenu(this) );
	setPopupMode( QToolButton::MenuButtonPopup );
	connect( _menu, SIGNAL(aboutToShow()), this, SLOT(showButtons()) );
	connect( _menu, SIGNAL(aboutToHide()), this, SLOT(hideButtons()) );
	connect( _groupBox, SIGNAL( buttonClicked( QAbstractButton * ) ), 
             this, SLOT( hideButtons( QAbstractButton * ) ) );
	connect( this, SIGNAL(triggered(QAction*)), this, SLOT(handleTriggered(QAction*)) );
	
	// Action for our button menu as icon or nothing will be seen
	QAction *action = new QAction(this);
	action->setCheckable( true );
	action->setVisible( true ); 
	setDefaultAction( action );
    
	// Actual positions of the buttons in the button menu layout */
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
void CAMenuToolButton::addButton( const QIcon icon, int buttonId ) {
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
*/	
void CAMenuToolButton::showButtons() { 
	QWidget *parent = _groupBox->parentWidget();
	
	// This code should move the menu position to the left
	// but it does not work, as QMenu sets it's own position
	// after showButtons method call
	if( parent )
	{
		QDesktopWidget *desktop = QApplication::desktop();
	    int desktopWidth = desktop->width();
	    int desktopHeight = desktop->height();
		if( parent->pos().x() + parent->width() > desktopWidth )
			parent->move( pos().x() - parent->width(), parent->pos().y() );
		printf("1 Pos: %d, %d, DW %d, BMW %d, Box x %d y %d w %d h %d\n",
		       parent->pos().x(), parent->pos().y(), desktopWidth, parent->width(),
		       _groupBox->x(), _groupBox->y(), _groupBox->width(), _groupBox->height() );
		fflush( stdout );
		parent->show();
		parent->updateGeometry();
	}
	_groupBox->updateGeometry();
	_groupBox->show();
}

/*!
	Hides the button menu and emits the triggered() signal.
*/
void CAMenuToolButton::hideButtons( QAbstractButton *button ) {
	_groupBox->hide();
	if (button) {
		setIcon(button->icon());
		setChecked( true );
		setCurrentId( _buttonGroup->id(button) );
		emit toggled( true, _buttonGroup->id(button) );
	}
}

/*!
	Emits triggered( bool, int ) signal.
*/
void CAMenuToolButton::handleTriggered( QAction *action ) {
	emit toggled( isChecked(), currentId() );
}

/*!
	\fn void CAButtonMenu::triggered( bool checked, int id )
	
	Signal sent when the button is clicked or an element is selected and changed.
*/	
