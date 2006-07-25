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

protected:
  /**
   * Initialize the toolbar
   * 
   */
  void initToolBar();

private:
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
