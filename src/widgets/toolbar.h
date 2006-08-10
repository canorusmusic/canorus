/** @file toolbar.h
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

#ifndef CNTOOLBAR_H
#define CNTOOLBAR_H

#include <QToolBar>
#include <QMenu>
#include <QToolButton>
#include <QHash>

class QGroupBox;
class QButtonGroup;
class QGridLayout;
class QAction;
class CAButtonMenu;

typedef enum
{
	CTB_Menu        = 0,
	CTB_Buttonmenu,
	CTB_Button,
	CTB_Combobox
} CTB_Type;

class CAToolBar : public QToolBar
{
	Q_OBJECT
public:
	/**
	 * Constructs the toolbar
	 * 
	 * @param oTitle    Toolbar title
	 * @param poParent  parent widget
	 *
	 */
	CAToolBar( const QString &oTitle, QWidget *poParent = 0 );
	CAToolBar( QWidget *poParent = 0 );
	/**
	 * Destructs the toolbar
	 * 
	 */
	~CAToolBar();
	/**
	 * Add Tool button with a menu to the toolbar
	 * 
	 * @param oTitle    Toolbutton title
	 * @param oName        Unique name to identify the toolbar element
	 * @param poMenu    Menu at the side of the tool button (including icons)
	 * @param poIcon    Icon of the tool button
	 * @param bToggle   'true': tool button is a toggle button
	 *
	 */
	void addToolMenu( const QString oTitle, QString oName, QMenu *poMenu,
	                  const QIcon *poIcon, bool bToggle = false );
	/**
	 * Add Tool button with a menu to the toolbar
	 * 
	 * @param oTitle        Toolbutton title
	 * @param oName        Unique name to identify the toolbar element
	 * @param poButtonGroup Button Group instead of normal menu (including icons)
	 * @param poIcon        Icon of the tool button
	 * @param bToggle       'true': tool button is a toggle button
	 *
	 */
	void addToolMenu( const QString oTitle, QString oName, CAButtonMenu *poButtonMenu, 
	                  const QIcon *poIcon, bool bToggle = false );
	/**
	 * Add a Tool button
	 * 
	 * @param oTitle       Toolbutton title
	 * @param oName        Unique name to identify the toolbar element
	 * @param poIcon       Icon of the tool button
	 * @param bToggle      'true': tool button is a toggle button
	 *
	 */
	void addToolButton( const QString oTitle, QString oName, const QIcon *poIcon,
	                    bool bToggle = false );
	/**
	 * Add a combobox menu
	 * 
	 * @param oTitle       Combobox title
	 * @param oName        Unique name to identify the toolbar element
	 * @param poItemList   List of combobox menu entries
	 * @param iIndex       Index of the menu entry to be displayed
	 *
	 */
	void addComboBox( QString oTitle, QString oName, QStringList *poItemList, 
	                  int iIndex );

signals:
	/**
	 * Signal sent when buttons are hidden with the selected button
	 * 
	 * @param poButton   selected button from the button menu
	 *
	 */	
    void buttonClicked( QAbstractButton *poButton );
    
protected:
	/**
	 * Initialize the toolbar
	 * 
	 */
	void initToolBar();
  
protected slots:
	/**
	 * Change the icon of a button menu
	 * 
	 * @param poButton       Button with the needed icon for update
	 *
	 */
	void changeMenuIcon( QAbstractButton *poButton );

private:
	QList<CTB_Type>	moToolTypes;	/// list with types of the toolbar elements
	QList<QWidget*>	moToolElements; /// list with the toolbar elements themself
	QHash<QString, int> moToolIDs;  /// hash of IDs to find the button icon to be shown
	QList<QAction*>	moToolActions;  /// list with the actions of the toolbar elements
	CAButtonMenu *mpoClefMenu;      /// menu for selection of a clef
	CAButtonMenu *mpoNoteMenu;      /// menu for the selection of a note length
	QMenu *mpoKeysigMenu;           /// menu for selection of a key signature
};

// Button Group shown as Menu
class CAButtonMenu : public QMenu
{
	Q_OBJECT
public:
	/**
	 * Constructs the button menu
	 * 
	 * @param oTitle    button menu title
	 * @param poParent  parent widget
	 *
	 */
	CAButtonMenu( QString oTitle, QWidget * poParent = 0 );
	/**
	 * Destructs the button menu
	 * 
	 */
	~CAButtonMenu();

	/**
	 * Add a Tool button to the menu
	 * 
	 * @param oIcon       Icon of the tool button
	 *
	 */
	void addButton( const QIcon &oIcon );
	/**
	 * Return the number of icons per row
	 * 
	 * @return number of icons per row
	 *
	 */
	int  getNumIconsPerRow();
	/**
	 * Set the number of icons per row
	 * 
	 * @param iNumIconsRow   number of icons per row
	 *
	 */	
	void setNumIconsPerRow( int iNumIconsRow );

public slots:
	/**
	 * Shows the button menu (connected to aboutToShow signal)
	 * 
	 */	
	void showButtons();
	/**
	 * Hides the button menu (connected to buttonClicked signal)
	 * 
	 */	
	void hideButtons( QAbstractButton *poButton );

signals:
	/**
	 * Signal sent when buttons are hidden with the selected button
	 * 
	 * @param poButton   selected button from the button menu
	 *
	 */	
    void buttonClicked( QAbstractButton *poButton );
    
protected:
	QButtonGroup *mpoBGroup;		/// Abstract group for the button actions
	QGroupBox    *mpoBBox;			/// Group box containing title and buttons
	QGridLayout  *mpoMLayout;       /// Layout for the group box
	QGridLayout  *mpoBLayout;       /// Layout for the button menu
	QList<QToolButton*> moButtons; 
	int            miBXPos;         /// X position of next button
	int            miBYPos;         /// Y position of next button
	int            miNumIconsRow;   /// Number of icons per row
	int            miSpace;         /// Space between buttons
	int            miMargin;        /// Margin of layout
};

#endif /* CNTOOLBAR_H */
