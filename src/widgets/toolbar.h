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

#include <QtGui/QToolBar>

class QMenu;
class QToolButton;
class QAction;

typedef enum
{
	CTB_Menu,
	CTB_Buttonmenu,
	CTB_Combobox
} CTB_Type;

class CAToolBar : public QToolBar
{
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
   * @param poMenu    Menu at the side of the tool button (including icons)
   * @param poIcon    Icon of the tool button
   * @param bToggle   'true': tool button is a toggle button
   *
   */
  void addToolMenu( const QString oTitle, QMenu *poMenu,
                    const QIcon *poIcon, bool bToggle = false );
  /**
   * Add Tool button with a menu to the toolbar
   * 
   * @param oTitle        Toolbutton title
   * @param poButtonGroup Button Group instead of normal menu (including icons)
   * @param poIcon        Icon of the tool button
   * @param bToggle       'true': tool button is a toggle button
   *
   */
  void addToolMenu( const QString oTitle, const QButtonGroup *poButtonGroup, 
                    const QIcon *poIcon, bool bToggle = false );
  /**
   * Add a Tool button
   * 
   * @param oTitle       Toolbutton title
   * @param poIcon       Icon of the tool button
   * @param bToggle      'true': tool button is a toggle button
   *
   */
  void addToolButton( const QString oTitle, const QIcon *poIcon,
                      bool bToggle = false );
  /**
   * Add a combobox menu
   * 
   * @param oTitle       Combobox title
   * @param iIndex       Index of the item to be displayed
   *
   */
  void addComboBox( QString oTitle, QList<QString*> oItemList, int iIndex );

protected:
  /**
   * Initialize the toolbar
   * 
   */
  void initToolBar();

private:
	QList<CTB_Type>	moToolTypes;
	QList<QWidget*>	moToolElements;
	QList<QAction*>	moToolActions;
  QToolButton *mpoClefButton;
  QToolButton *mpoNoteButton;
  QToolButton *mpoKeysigButton;
  QMenu *mpoClefMenu;
  QMenu *mpoNoteMenu;
  QMenu *mpoKeysigMenu;
  QAction *mpoClefAction;
  QAction *mpoNoteAction;
  QAction *mpoKeysigAction;
};

#endif /* CNTOOLBAR_H */
