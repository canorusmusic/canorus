/** @file widgets/toolbar.cpp
 *
 * This program is free software; you can redistribute it and/or modify it   
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; version 2 of the License.                       
 *                                                                           
 * This program is distributed in the hope that it will be useful, but       
 * WITHOUT ANY WARRANTY; without even the implied warranty of               
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General  
 * Public License for more details.                                          
 *                                                                           
 * You should have received a copy of the GNU General Public License along   
 * with this program; (See "LICENSE.GPL"). If not, write to the Free         
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA        
 * 02111-1307, USA.                                                          
 *                                                                           
 *---------------------------------------------------------------------------
 *                                                                           
 * Copyright (c) 2006, Reinhard Katzmann, Canorus development team           
 * All Rights Reserved. See AUTHORS for a complete list of authors.          
 *                                                                           
 */

#include <QMenu>
#include <QPushButton>
#include <QComboBox>
#include <QButtonGroup>
#include <QPushButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QApplication>
#include <QDesktopWidget>

#include "toolbar.h"

CAToolBar::CAToolBar( const QString &oTitle, QWidget *poParent /* = 0 */ )
  : QToolBar( oTitle, poParent )
{
}

CAToolBar::CAToolBar( QWidget *poParent /* = 0 */ )
  : QToolBar( poParent )
{
}

CAToolBar::~CAToolBar()
{
}

void CAToolBar::changeMenuIcon( QAbstractButton *poButton )
{
	int iBMID = moToolIDs[poButton->objectName()];
	int iBID  = 0;
	// Read the associated toolbar button from the list
	QToolButton  *poToolButton = (QToolButton *)(moToolElements[iBMID]);
	// Read corresponding menu
	CAButtonMenu *poButtonMenu = (CAButtonMenu *)poToolButton->menu();
	// Set the new icon from the selected button
	poToolButton->setIcon( poButton->icon() );
	// Set the buttons button ID as selected button ID
	iBID = poButtonMenu->getButtonID( poButton );
	printf("changeMenuIcon: BMID: %x/%s/%d ID %d\n", poButton, 
	        poButton->objectName().toAscii().constData(), iBMID, iBID);
	fflush(stdout);
	// Set the button ID as selected button element
	moSelectedIDs[iBMID] = QVariant( iBID );
	// Notify others about the new selection
	emit buttonClicked( poToolButton );
	emit actionTriggered( moToolActions[iBMID] );
}

void CAToolBar::menuEntryChanged( QAction *poAction )
{
	int iBMID = moToolIDs[poAction->objectName()];
	// Set the Action text as selected menu item
	moSelectedIDs[iBMID] = QVariant( poAction->text() );	
	// Notify others about the new selection
	emit actionTriggered( moToolActions[iBMID] );
	connect( this, SIGNAL( menuElemSelected( bool ) ),
		     moToolActions[iBMID], SIGNAL( toggled( bool ) ) );
    emit menuElemSelected( moToolActions[iBMID]->isChecked() );
	disconnect( this, SIGNAL( menuElemSelected( bool ) ),
		        moToolActions[iBMID], SIGNAL( toggled( bool ) ) );
}

QAction *CAToolBar::addToolMenu( const QString oTitle, QString oName, QMenu *poMenu,
                                 const QIcon *poIcon, bool bToggle /* = false */ )
{
	// ToDo: Could be improved by calling addToolButton
	QToolButton  *poMenuButton = 0;
	QAction      *poAction     = 0;
	int          iSize        = moToolIDs.size();
	moToolIDs[oName] = moToolElements.size();
	// Check if name was unique (i.e. new hash inserted)
	if( iSize == moToolIDs.size() )
	{
		printf("CATB-Error: ToolMenu name %s not unique!\n", 
		       oName.toAscii().constData ());
		fflush(stdout);
		return 0;
	}
	moSelectedIDs.append( QVariant( QString() ) );
	moToolTypes.append(CTB_Menu);
	// Create new Toolbutton
	poMenuButton = new QToolButton();
	// Add it to the list of elements for saving it later
	if( poMenuButton )
	{
		moToolElements.append(poMenuButton);
		poMenuButton->setObjectName( oName );
		poMenu->setObjectName( oName );
		// Update icon
		poMenuButton->setIcon( *poIcon );
		// Toggle button ?
		poMenuButton->setCheckable( bToggle );
		// Tooltip
		poMenuButton->setToolTip( oTitle );
		// Add action to the element
		poAction = addWidget( poMenuButton );
		poAction->setObjectName( oName );
		// Save it in the action list
		moToolActions.append( poAction );
		// Associate menu with the button
		poMenuButton->setMenu( poMenu );
		// Add menu grab
		poMenuButton->setPopupMode( QToolButton::MenuButtonPopup );
		// Notify on toggle of Tool Button
		connect( poMenuButton, SIGNAL( toggled( bool ) ),
			     poAction, SIGNAL( toggled( bool ) ) );
		// Notify on toggle of Action
		connect( poAction, SIGNAL( toggled( bool ) ),
			     poMenuButton, SLOT( setChecked( bool ) ) );
		// Notify on change of menu entry
	    connect( poMenu, SIGNAL( triggered( QAction * ) ),
	    		 this, SLOT( menuEntryChanged( QAction * ) ) );
	}
	return poAction;
}

QAction *CAToolBar::addToolMenu( const QString oTitle, QString oName, CAButtonMenu *poButtonMenu, 
                                 const QIcon *poIcon, bool bToggle /* = false */ )
{
	// ToDo: Could be improved by calling addToolButton
	QToolButton *poMenuButton = 0;
	QAction     *poAction     = 0;
	int          iSize        = moToolIDs.size();
	moToolIDs[oName] = moToolElements.size();
	// Check if name was unique (i.e. new hash inserted)
	if( iSize == moToolIDs.size() )
	{
		printf("CATB-Error: ToolButtonMenu name %s not unique!\n", 
		       oName.toAscii().constData ());
		fflush(stdout);
		return 0;
	}
	moSelectedIDs.append( QVariant( 0 ) );
	moToolTypes.append(CTB_Menu);
	// Create new Toolbutton
	poMenuButton = new QToolButton();
	// Add it to the list of elements for saving it later
	if( poMenuButton )
	{
		moToolElements.append(poMenuButton);
		poMenuButton->setObjectName( oName );
		poButtonMenu->setObjectName( oName );
		// Update icon
		poMenuButton->setIcon( *poIcon );
		// Toggle button ?
		poMenuButton->setCheckable( bToggle );
		// Tooltip
		poMenuButton->setToolTip( oTitle );
		// Add action to the element
		poAction = addWidget( poMenuButton );
		poAction->setObjectName( oName );
		// Save it in the action list
		moToolActions.append( poAction );
		// Associate menu with the button
		poMenuButton->setMenu( poButtonMenu );
		// Add menu grab
		poMenuButton->setPopupMode( QToolButton::MenuButtonPopup );
		// Notify on toggle of Tool Button
		connect( poMenuButton, SIGNAL( toggled( bool ) ),
			     poAction, SIGNAL( toggled( bool ) ) );
		// Notify on toggle of Action
		connect( poAction, SIGNAL( toggled( bool ) ),
			     poMenuButton, SLOT( setChecked( bool ) ) );
		// Notify on change of button
		connect( poButtonMenu, SIGNAL( buttonClicked( QAbstractButton * ) ),
			     this, SLOT( changeMenuIcon( QAbstractButton * ) ) );
		connect( poButtonMenu, SIGNAL( buttonElemToggled( bool ) ),
			     poMenuButton, SIGNAL( toggled( bool ) ) );
	}
	return poAction;
}

QAction *CAToolBar::addToolButton( const QString oTitle, QString oName, 
                                   const QIcon *poIcon, bool bToggle /* = false */ )
{
	QToolButton *poMenuButton = 0;
	QAction     *poAction     = 0;
	int          iSize        = moToolIDs.size();
	moToolIDs[oName] = moToolElements.size();
	// Check if name was unique (i.e. new hash inserted)
	if( iSize == moToolIDs.size() )
	{
		printf("CATB-Error: ToolButton name %s not unique!\n", 
		       oName.toAscii().constData());
		fflush(stdout);
		return 0;
	}
	moToolTypes.append(CTB_Button);
	// Create new Toolbutton
	poMenuButton = new QToolButton();
	// Add it to the list of elements for saving it later
	if( poMenuButton )
	{
		moToolElements.append(poMenuButton);
		poMenuButton->setObjectName( oName );
		// Update icon
		poMenuButton->setIcon( *poIcon );
		// Toggle button ?
		poMenuButton->setCheckable( bToggle );
		// Tooltip
		poMenuButton->setToolTip( oTitle );
		poAction->setObjectName( oName );
		// Add action to the element
		poAction = addWidget( poMenuButton );
		// Save it in the action list
		moToolActions.append( poAction );
		// Notify on toggle of Tool Button
		connect( poMenuButton, SIGNAL( toggled( bool ) ),
			     poAction, SIGNAL( toggled( bool ) ) );
		// Notify on toggle of Action
		connect( poAction, SIGNAL( toggled( bool ) ),
			     poMenuButton, SLOT( setChecked( bool ) ) );
	}
	return poAction;
}

QAction *CAToolBar::addComboBox( QString oTitle, QString oName, 
                                  QStringList *poItemList, int iIndex )
{
	QComboBox *poComboMenu = 0;
	QAction   *poAction    = 0;
	int          iSize        = moToolIDs.size();
	moToolIDs[oName] = moToolElements.size();
	// Check if name was unique (i.e. new hash inserted)
	if( iSize == moToolIDs.size() )
	{
		printf("CATB-Error: ToolComboBox name %s not unique!\n",
		       oName.toAscii().constData());
		fflush(stdout);
		return 0;
	}
	moSelectedIDs.append( QVariant( 0 ) );
	moToolTypes.append(CTB_Combobox);
	// Create new Combobox Menu
	poComboMenu = new QComboBox();
	// Add it to the list of elements for saving it later
	if( poComboMenu )
	{
		moToolElements.append(poComboMenu);
		poComboMenu->setObjectName( oName );
		// Tooltip
		poComboMenu->setToolTip( oTitle );
		// Add action to the element
		poAction = addWidget( poComboMenu );
		poAction->setObjectName( oName );
		// Save it in the action list
		moToolActions.append( poAction );
		// Add menu entries
		for (int i = 0; i < poItemList->size(); ++i) 
			poComboMenu->addItems( *poItemList );
		poComboMenu->setCurrentIndex( iIndex );
	}
	return poAction;
}

bool CAToolBar::setAction( QString oName, QAction *poAction )
{
	bool bRet  = false;
	int  iSize = moToolIDs.size();
	int iBMID = moToolIDs[oName];
	// Check if oName was found in the hash (i.e. not inserted)
	if( iSize == moToolIDs.size() )
	{
		// Trying to add a second Action to the ToolButtons gives the error:
		// QToolButton: menu in setMenu() overriding actions set in addAction!
		//moToolElements[iBMID]->addAction( poAction );
		//poAction->setIcon( moToolActions[iBMID]->icon() );
		connect( poAction, SIGNAL( toggled( bool ) ), moToolActions[iBMID],
		         SIGNAL( toggled( bool ) ) );
		connect( poAction, SIGNAL( triggered( bool ) ), moToolActions[iBMID],
		         SIGNAL( toggled( bool ) ) );
		bRet = true;
	}
	else // Not found: Remove the wrongly inserted hash
		moToolIDs.remove( oName );
	return bRet;
}

QVariant CAToolBar::toolElemValue( QString oName )
{
	QVariant oVal( -1 );
	
	int iSize = moToolIDs.size();
	int iBMID = moToolIDs[oName];
	// Check if oName was found in the hash (i.e. not inserted)
	if( iSize == moToolIDs.size() )
	{
		switch( moToolTypes[iBMID] )
		{
			case CTB_Menu:
				oVal = moSelectedIDs[iBMID];
				break;
			case CTB_Buttonmenu:
				oVal = moSelectedIDs[iBMID];
				break;
			case CTB_Button: // Nothing can be selected here
				oVal = QVariant( 0 );
				break;
			case CTB_Combobox:
			{
				QComboBox *poComboMenu = (QComboBox *)moToolElements[iBMID];
				oVal = QVariant( poComboMenu->currentIndex() );
				//oVal = QVariant( poComboMenu->currentText() );
			}
			break;
		}
	}
	else // Not found: Remove the wrongly inserted hash
		moToolIDs.remove( oName );
	return oVal;
}

CAButtonMenu::CAButtonMenu( QString oTitle, QWidget * poParent /* = 0 */ ) 
	: QMenu( poParent )
{ 
	miMargin      = 5;
	miSpace       = 4;
	miNumIconsRow = 4;
	// Abstract group for mutual exclusive toggle
	mpoBGroup = new QButtonGroup( this );
	// Size policy: Expanding / Expanding
    QSizePolicy oSizePolicy(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(7));
    oSizePolicy.setHorizontalStretch(0);
    oSizePolicy.setVerticalStretch(0);
    QSizePolicy oBMSPol = sizePolicy();
    oSizePolicy.setHeightForWidth( oBMSPol.hasHeightForWidth() );
	// Layout for visual group box
	mpoMLayout = new QGridLayout( 0 );
	mpoMLayout->setSpacing( 0 );
    mpoMLayout->setMargin( 0 );
    setSizePolicy( oSizePolicy );
    // Visual group box for the button menu
	mpoBBox   = new QGroupBox( oTitle, this );
	mpoBBox->setFlat(true);
    oSizePolicy.setHeightForWidth(mpoBBox->sizePolicy().hasHeightForWidth());
    mpoBBox->setSizePolicy( oSizePolicy );
	// Layout for button menu
	mpoBLayout = new QGridLayout( mpoBBox );
	mpoBLayout->setSpacing( miSpace );
    mpoBLayout->setMargin( miMargin );
	// We only show the button menu when it is time to do it
	connect( this, SIGNAL( aboutToShow() ), this, SLOT( showButtons() ) );
	// Actual positions of the buttons in the button menu layout
	miBXPos = miBYPos = 0;
	QIcon oBMIcon;
	mpoBBox->setBackgroundRole( QPalette::Button );
	mpoBBox->setAutoFillBackground( true );
}

CAButtonMenu::~CAButtonMenu()
{
	for (int i = 0; i < moButtons.size(); ++i)
		delete moButtons[i];
	delete mpoBGroup;
	delete mpoBBox;
}

int CAButtonMenu::getNumIconsPerRow()
{
	return miNumIconsRow;
}

void CAButtonMenu::setNumIconsPerRow( int iNumIconsRow )
{
	miNumIconsRow = iNumIconsRow;
}

void CAButtonMenu::addButton( const QIcon &oIcon, int iButtonID )
{
	QToolButton *poMButton;
	QFontMetrics oMetrics ( mpoBBox->font() );
	int iIconSize = 24,          // Size of Icon
	    iXMargin  = miMargin * 2, // Margin around the buttons
	    iYMargin  = miMargin * 2 + oMetrics.height(), // includes the height of the menu text
	    iXSize    = iIconSize,
	    iYSize    = iIconSize;  
	// Create new button for menu
	poMButton = new QToolButton( mpoBBox );
	poMButton->setIcon( oIcon );
	poMButton->setIconSize( QSize(iIconSize, iIconSize) );
	poMButton->setCheckable( true );
	// Useful if you want to switch icons of an associated toolbar
	poMButton->setObjectName( objectName() );
	if( miBXPos == 0 )
	{
		// Action for our button menu as icon or nothing will be seen
		QAction *poAction = addAction( "" );
		poAction->setVisible( true ); 
		poMButton->setChecked( true );
		setDefaultAction( poAction );
	    // Connect Hide action to the button
	    connect( mpoBGroup, SIGNAL( buttonClicked( QAbstractButton * ) ), 
	             this, SLOT( hideButtons( QAbstractButton * ) ) );
	}
	moButtons.append( poMButton );
	// Add it to the abstract group
    mpoBGroup->addButton( moButtons.last(), iButtonID );
    // Create menu that has miNumIconsRow buttons in each row
    if( miBXPos >= miNumIconsRow )
    {
    	miBXPos = 0;
    	++miBYPos;
    }
    // Add it to the button menu layout
    mpoBLayout->addWidget( poMButton, miBYPos, miBXPos, Qt::AlignLeft );
    ++miBXPos;
    if( miBYPos > 0 )
    {
	    iXSize = miNumIconsRow * (miSpace+poMButton->width()/3);
		iYSize = (miBYPos+1)   * (miSpace+poMButton->height());
    }
    else
    {
	    iXSize = miBXPos * (miSpace+poMButton->width()/3);
		iYSize = (miBYPos+1) * (miSpace+poMButton->height());
    }
	//printf("XPos %d, MX: %d, SX: %d, YPos %d, MY: %d, SY: %d\n",
	//       miBXPos, iXMargin, iXSize, miBYPos, iYMargin, iYSize);
	//fflush( stdout );
	setMinimumSize( iXMargin + iXSize, iYMargin + iYSize );
}

void CAButtonMenu::showButtons()
{ 
	// This code should move the menu position to the left
	// but it does not work, as QMenu sets it's own position
	// after showButtons method call
	/*QDesktopWidget *poDesktop = QApplication::desktop();
    int iDWidth = poDesktop->width();
    int iDHeight = poDesktop->height();
	if( pos().x() + minimumSize().width() > iDWidth )
		move( pos().x() - minimumSize().width(), pos().y() );*/
	//printf("Pos: %d, %d, DW %d, BMW %d\n",
	//       pos().x(), pos().y(), iDWidth, minimumSize().width() );
	//fflush( stdout );
	mpoBBox->show();
	mpoBBox->updateGeometry();
}

void CAButtonMenu::hideButtons( QAbstractButton *poButton )
{
	hide();
	emit buttonClicked( poButton );
	emit buttonElemToggled( poButton->isChecked() );
}
