/** @file ui/keysigpsp.h
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

#ifndef KEYSIGPSP_H_
#define KEYSIGPSP_H_

#include <QDockWidget>

#include "ui_keysig.h"

class QTreeWidgetItem;

// Key signature perspective
class CAKeySigPSP : public QDockWidget
{
	Q_OBJECT
public:
	/**
	* Constructor for key signature perspective
	* 
	* @param  oTitle      Title of perspective
	* @param  poParent    parent widget
	* @param  eFlags      widget flags
	*
	*/
	CAKeySigPSP( const QString & oTitle, QWidget * poParent = 0, Qt::WFlags eFlags = 0 );
	virtual ~CAKeySigPSP();
	
	/**
	* Returns the currently selected key signature as integer value
	* 
	* @returns key signature as integer value (for normal key signatures)
	*
	*/
	int getKeySignature();
	
protected slots:
	/**
	* Updates the list box when the slider is moved
	* 
	* @param  iNewPos      New position of slider
	*
	*/
	void newKSSliderPos( int iNewPos );
	/**
	* Updates the slider when a new list box element is selected
	* 
	* @param  iNewItem       Selected item
	* @param  iNewColumn     Selected column
	*
	*/
	void newKSListViewItem( QTreeWidgetItem *iNewItem, int iNewColumn );
	
protected:
	Ui::KeySig       moKeySigWidget;  /// Key signature perspective represantative
	QMap<int,int>    moListSliderMap; /// Mapping: ListBox elements to slider position
};

#endif /*KEYSIGPSP_H_*/
