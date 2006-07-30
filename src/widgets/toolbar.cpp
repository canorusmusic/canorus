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

#include <QtGui/QMenu>
#include <QtGui/QToolButton>

#include "toolbar.h"

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
	mpoClefMenu   = new QMenu();
	mpoNoteMenu   = new QMenu();
	mpoKeysigMenu = new QMenu(); 
	QIcon oCIcon(  QString::fromUtf8(":/menu/images/cleftreble.png") );
	addToolMenu( "Clef", mpoClefMenu, &oCIcon, true );
	QIcon oNIcon(  QString::fromUtf8(":/menu/images/n4.png") );
	addToolMenu( "Note", mpoNoteMenu, &oNIcon, true );
	QIcon oDFIcon( QString::fromUtf8(":/menu/images/doubleflat.png") );
	addToolMenu( "Key Signature", mpoKeysigMenu, &oDFIcon, true );
	
	// Add all the menu entries, either as text or icons
	mpoClefMenu->addAction( "treble" );
	mpoClefMenu->addAction( "alto" );
	mpoClefMenu->addAction( "bass" );
	mpoNoteMenu->addAction( "half" );
	mpoNoteMenu->addAction( "quarter" );
	mpoNoteMenu->addAction( "eigth" );
	mpoKeysigMenu->addAction( "C" );
	mpoKeysigMenu->addAction( "G" );
	mpoKeysigMenu->addAction( "D" );
	mpoKeysigMenu->addAction( "A" );
	mpoKeysigMenu->addAction( "E" );
}

void CAToolBar::addToolMenu( const QString oTitle, QMenu *poMenu,
                             const QIcon *poIcon, bool bToggle /* = false */ )
{
	QToolButton *poMenuButton = 0;
	QAction     *poAction     = 0;
	moToolTypes.append(CTB_Menu);
	// Create new Toolbutton
	poMenuButton = new QToolButton();
	// Add it to the list of elements
	if( poMenuButton )
		moToolElements.append(poMenuButton);
	// Update icon
	poMenuButton->setIcon( *poIcon );
	// Add action to the element
	poAction = addWidget( poMenuButton );
	// Associate menu with the button
	poMenuButton->setMenu( poMenu );
	poMenuButton->setPopupMode( QToolButton::MenuButtonPopup );
}

void CAToolBar::addToolMenu( const QString oTitle, const QButtonGroup *poButtonGroup, 
                             const QIcon *oIcon, bool bToggle /* = false */ )
{
}

void CAToolBar::addToolButton( const QString oTitle, const QIcon *poIcon, 
                               bool bToggle /* = false */ )
{
}

void CAToolBar::addComboBox( QString oTitle, QList<QString*> oItemList,
                             int iIndex )
{
}
