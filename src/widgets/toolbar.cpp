/** @file toolbar.cpp
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
#include <QButtonGroup>
#include <QPushButton>
#include <QComboBox>
#include <QButtonGroup>
#include <QPushButton>
#include <QGroupBox>
#include <QGridLayout>

#include "widgets/toolbar.h"

CAToolBar::CAToolBar( const QString &oTitle, QWidget *poParent /* = 0 */ )
  : QToolBar( oTitle, poParent )
{
  initToolBar();
}

CAToolBar::CAToolBar( QWidget *poParent /* = 0 */ )
  : QToolBar( poParent )
{
  initToolBar();
}

CAToolBar::~CAToolBar()
{
  delete mpoClefMenu;
  delete mpoNoteMenu;
  delete mpoKeysigMenu;
}

void CAToolBar::initToolBar()
{
	// Test Code for Toolbar
	mpoClefMenu   = new CAButtonMenu( tr("Select Clef" ) );
	mpoNoteMenu   = new CAButtonMenu( tr("Select Note" ) );
	mpoKeysigMenu = new QMenu(); 
	
	QIcon oClefTrebleIcon( QString::fromUtf8(":/menu/images/cleftreble.png") );
	QIcon oClefAltoIcon(   QString::fromUtf8(":/menu/images/clefalto.png") );
	QIcon oClefBassIcon(   QString::fromUtf8(":/menu/images/clefbass.png") );
	
	QIcon oN0Icon(  QString::fromUtf8(":/menu/images/n0.png") );
	QIcon oN1Icon(  QString::fromUtf8(":/menu/images/n1.png") );
	QIcon oN2Icon(  QString::fromUtf8(":/menu/images/n2.png") );
	QIcon oN4Icon(  QString::fromUtf8(":/menu/images/n4.png") );
	QIcon oN8Icon(  QString::fromUtf8(":/menu/images/n8.png") );
	QIcon oN16Icon(  QString::fromUtf8(":/menu/images/n16.png") );
	QIcon oN32Icon(  QString::fromUtf8(":/menu/images/n32.png") );
	QIcon oN64Icon(  QString::fromUtf8(":/menu/images/n64.png") );
	
	addToolMenu( "Clef", mpoClefMenu, &oClefTrebleIcon, true );
	addToolMenu( "Note", mpoNoteMenu, &oN4Icon, false );
	QIcon oDFIcon( QString::fromUtf8(":/menu/images/doubleflat.png") );
	addToolMenu( "Key Signature", mpoKeysigMenu, &oDFIcon, true );
	
	// Add all the menu entries, either as text or icons
	mpoClefMenu->addButton( oClefTrebleIcon );
	mpoClefMenu->addButton( oClefAltoIcon );
	mpoClefMenu->addButton( oClefBassIcon );
	connect( mpoClefMenu, SIGNAL( buttonClicked( QAbstractButton * ) ),
		     this, SLOT( changeClefMenuIcon( QAbstractButton * ) ) );
	//mpoNoteMenu->addAction( "half" );
	//mpoNoteMenu->addAction( "quarter" );
	//mpoNoteMenu->addAction( "eigth" );
	printf("Adding 0\n"); fflush(stdout);
	mpoNoteMenu->addButton( oN0Icon );
	printf("Adding 1\n"); fflush(stdout);
	mpoNoteMenu->addButton( oN1Icon );
	printf("Adding 2\n"); fflush(stdout);
	mpoNoteMenu->addButton( oN2Icon );
	printf("Adding 4\n"); fflush(stdout);
	mpoNoteMenu->addButton( oN4Icon );
	printf("Adding 8\n"); fflush(stdout);
	mpoNoteMenu->addButton( oN8Icon );
	printf("Adding 16\n"); fflush(stdout);
	mpoNoteMenu->addButton( oN16Icon );
	printf("Adding 32\n"); fflush(stdout);
	mpoNoteMenu->addButton( oN32Icon );
	printf("Adding 64\n"); fflush(stdout);
	mpoNoteMenu->addButton( oN64Icon );
	connect( mpoNoteMenu, SIGNAL( buttonClicked( QAbstractButton * ) ),
		     this, SLOT( changeNoteMenuIcon( QAbstractButton * ) ) );
	mpoKeysigMenu->addAction( "C" );
	mpoKeysigMenu->addAction( "G" );
	mpoKeysigMenu->addAction( "D" );
	mpoKeysigMenu->addAction( "A" );
	mpoKeysigMenu->addAction( "E" );
}

void CAToolBar::changeNoteMenuIcon( QAbstractButton *poButton )
{
	((QToolButton *)(moToolElements[1]))->setIcon( poButton->icon() );
}

void CAToolBar::changeClefMenuIcon( QAbstractButton *poButton )
{
	((QToolButton *)(moToolElements[0]))->setIcon( poButton->icon() );
}

void CAToolBar::addToolMenu( const QString oTitle, QMenu *poMenu,
                             const QIcon *poIcon, bool bToggle /* = false */ )
{
	// ToDo: Could be improved by calling addToolButton
	QToolButton *poMenuButton = 0;
	QAction     *poAction     = 0;
	moToolTypes.append(CTB_Menu);
	// Create new Toolbutton
	poMenuButton = new QToolButton();
	// Add it to the list of elements for saving it later
	if( poMenuButton )
		moToolElements.append(poMenuButton);
	// Update icon
	poMenuButton->setIcon( *poIcon );
	// Toggle button ?
	poMenuButton->setCheckable( bToggle );
	// Tooltip
	poMenuButton->setToolTip( oTitle );
	// Add action to the element
	poAction = addWidget( poMenuButton );
	// Save it in the action list
	moToolActions.append( poAction );
	// Associate menu with the button
	poMenuButton->setMenu( poMenu );
	poMenuButton->setPopupMode( QToolButton::MenuButtonPopup );
}

void CAToolBar::addToolMenu( const QString oTitle, CAButtonMenu *poButtonMenu, 
                             const QIcon *poIcon, bool bToggle /* = false */ )
{
	// ToDo: Could be improved by calling addToolButton
	QToolButton *poMenuButton = 0;
	QAction     *poAction     = 0;
	moToolTypes.append(CTB_Menu);
	// Create new Toolbutton
	poMenuButton = new QToolButton();
	// Add it to the list of elements for saving it later
	if( poMenuButton )
		moToolElements.append(poMenuButton);
	// Update icon
	poMenuButton->setIcon( *poIcon );
	// Toggle button ?
	poMenuButton->setCheckable( bToggle );
	// Tooltip
	poMenuButton->setToolTip( oTitle );
	// Add action to the element
	poAction = addWidget( poMenuButton );
	// Save it in the action list
	moToolActions.append( poAction );
	// Associate menu with the button
	poMenuButton->setMenu( poButtonMenu );
	poMenuButton->setPopupMode( QToolButton::MenuButtonPopup );
}

void CAToolBar::addToolButton( const QString oTitle, const QIcon *poIcon, 
                               bool bToggle /* = false */ )
{
	QToolButton *poMenuButton = 0;
	QAction     *poAction     = 0;
	moToolTypes.append(CTB_Button);
	// Create new Toolbutton
	poMenuButton = new QToolButton();
	// Add it to the list of elements for saving it later
	if( poMenuButton )
		moToolElements.append(poMenuButton);
	// Update icon
	poMenuButton->setIcon( *poIcon );
	// Toggle button ?
	poMenuButton->setCheckable( bToggle );
	// Tooltip
	poMenuButton->setToolTip( oTitle );
	// Add action to the element
	poAction = addWidget( poMenuButton );
	// Save it in the action list
	moToolActions.append( poAction );
}

void CAToolBar::addComboBox( QString oTitle, QStringList *poItemList,
                             int iIndex )
{
	QComboBox *poComboMenu = 0;
	QAction   *poAction    = 0;
	moToolTypes.append(CTB_Combobox);
	// Create new Combobox Menu
	poComboMenu = new QComboBox();
	// Add it to the list of elements for saving it later
	if( poComboMenu )
		moToolElements.append(poComboMenu);
	// Tooltip
	poComboMenu->setToolTip( oTitle );
	// Add action to the element
	poAction = addWidget( poComboMenu );
	// Save it in the action list
	moToolActions.append( poAction );
	// Add menu entries
	for (int i = 0; i < poItemList->size(); ++i) 
		poComboMenu->addItems( *poItemList );
	poComboMenu->setCurrentIndex( iIndex );
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

void CAButtonMenu::addButton( const QIcon &oIcon )
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
	if( miBXPos == 0 )
	{
		// Action for our button menu as icon or nothing will be seen
		QAction *poAction = addAction( "" );
		poAction->setVisible( true ); 
		poMButton->setChecked( true );
		setDefaultAction( poAction );
	}
	moButtons.append( poMButton );
	// Add it to the abstract group
    mpoBGroup->addButton( moButtons.last() );
    // Connect Hide action to the button
    connect( mpoBGroup, SIGNAL( buttonClicked( QAbstractButton * ) ), 
             this, SLOT( hideButtons( QAbstractButton * ) ) );
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
	printf("XPos %d, MX: %d, SX: %d, YPos %d, MY: %d, SY: %d\n",
	       miBXPos, iXMargin, iXSize, miBYPos, iYMargin, iYSize);
	fflush( stdout );
	setMinimumSize( iXMargin + iXSize, iYMargin + iYSize );
}

void CAButtonMenu::showButtons()
{ 
	mpoBBox->show();
	mpoBBox->updateGeometry();
	//printf("Pos: %d, %d\n",pos().x(), pos().y() );
	//fflush( stdout );
	//move( iX, iY );
}

void CAButtonMenu::hideButtons( QAbstractButton *poButton )
{
	hide();
	emit buttonClicked( poButton );
}
