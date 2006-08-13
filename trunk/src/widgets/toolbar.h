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
#include <QButtonGroup>

class QGroupBox;
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
	QAction *addToolMenu( const QString oTitle, QString oName, QMenu *poMenu,
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
	QAction *addToolMenu( const QString oTitle, QString oName, CAButtonMenu *poButtonMenu, 
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
	QAction *addToolButton( const QString oTitle, QString oName, const QIcon *poIcon,
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
	QAction *addComboBox( QString oTitle, QString oName, QStringList *poItemList, 
	                      int iIndex );
	                      
	/**
	 * Sets the action to a toolbar element
	 * 
	 * @param oName        Unique name to identify the toolbar element
	 * @param poAction     additional action
	 * @return             'true': Name was found in the list of toolbar elements
	 *
	 */
	bool setAction( QString oName, QAction *poAction );

	/**
	 * Returns the current value of a tool element
	 * 
	 * @param oName        Unique name to identify the toolbar element
	 * @return             value of a tool element
	 *
	 */
	 QVariant toolElemValue( QString oName );

signals:
	/**
	 * Signal sent when buttons are hidden with the selected button
	 * 
	 * @param poButton   selected button from the button menu
	 *
	 */	
    void buttonClicked( QAbstractButton *poButton );
	/**
	 * Signal sent when menu entry was selected
	 * 
	 * @param bChanged  'true': Tool button is activated
	 * 
	 */
	void menuElemSelected( bool bChecked );
    
protected slots:
	/**
	 * Change the icon of a button menu
	 * 
	 * @param poButton       Button with the needed icon for update
	 *
	 */
	void changeMenuIcon( QAbstractButton *poButton );
	void menuEntryChanged( QAction *poAction );
	
private:
	QList<CTB_Type>	moToolTypes;	/// list with types of the toolbar elements
	QList<QWidget*>	moToolElements; /// list with the toolbar elements themself
	QHash<QString, int> moToolIDs;  /// hash of IDs to find the button icon to be shown
	QList<QVariant> moSelectedIDs;  /// list of IDs of selected toolbar elements
	QList<QAction*>	moToolActions;  /// list with the actions of the toolbar elements
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
	void addButton( const QIcon &oIcon, int iButtonID );
	/**
	 * Return the number of icons per row
	 * 
	 * @param iNumIconsRow   number of icons per row
	 *
	 */
	int  getNumIconsPerRow();
	/**
	 * Return the button ID of a button
	 * 
	 * @param poButton   button whose ID should be returned
	 * @return           button ID of poButton
	 *
	 */
	int  getButtonID( QAbstractButton *poButton )
	{ return mpoBGroup->id( poButton ); }
	/**
	 * Set the number of icons per row
	 * 
	 * @return number of icons per row
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
	/**
	 * Signal sent when state of tool button is changed
	 * 
	 * @param bChanged  'true': Tool button is activated
	 * 
	 */
	void buttonElemToggled( bool bChecked );
        
protected:
	QButtonGroup *mpoBGroup;		/// Abstract group for the button actions
	QGroupBox    *mpoBBox;			/// Group box containing title and buttons
	QGridLayout  *mpoMLayout;       /// Layout for the group box
	QGridLayout  *mpoBLayout;       /// Layout for the button menu
	QList<QToolButton*> moButtons; 
	QHash<QString, int> moToolIDs;  /// hash of IDs of buttons
	int            miBXPos;         /// X position of next button
	int            miBYPos;         /// Y position of next button
	int            miNumIconsRow;   /// Number of icons per row
	int            miSpace;         /// Space between buttons
	int            miMargin;        /// Margin of layout
};

#endif /* CNTOOLBAR_H */
