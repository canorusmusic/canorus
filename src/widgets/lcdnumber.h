/** @file numberdisplay.h
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

#ifndef NUMBERDISPLAY_H

#define NUMBERDISPLAY_H

#include <QLCDNumber> 

class QAction;

// LCD Number enhanced with the possibility to limit the number displayed in the LCD
class CALCDNumber : public QLCDNumber 
{
	Q_OBJECT

public:
	/**
	* Constructs the number display
	* 
	* @param iMin         minimal number
	* @param iMax         maximal number
	* @param oToolTipText text to be displayed as tool tip    
	* @param oShortCut    shortcut for a quick change of the number
	* @param poParent     parent widget
	* @param oText        name of the widget
	*
	*/
	CALCDNumber (int iMin, int iMax, QWidget * poParent=0, QString oText = "");

	/**
	* Sets the current value of the number display
	* 
	* @param iVal         new value to be set
	*
	*/
	void setRealValue(int val);
	/**
	* Gets the current value of the number display
	* 
	*/
	int getRealValue();
	/**
	* Sets the minimum value of the number display
	* 
	* @param iMin         new minimum value to be set
	*
	*/
	void setMin(int iMin);
	/**
	* Sets the maximum value of the number display
	* 
	* @param iMax         new maximum value to be set
	*
	*/
	void setMax(int iMax);
	/**
	* Checks if the value is 0 (or false if LCD number wasn't plugged)
	* 
	*/
	bool isZero();
signals:
	/**
	* Actually sends out the changed value if triggered via mouse press event
	* 
	* @param iVal     new value
	*
	*/
	void valChanged(int iVal);
protected:
	/**
	* Event being performed when a mouse button was pressed
	* 
	* @param poEvt         Necessary information about the event
	*
	*/
	virtual void mousePressEvent(QMouseEvent *poEvt); 
	/**
	* Event being performed when a wheel was moved
	* 
	* @param poEvt         Necessary information about the event
	*
	*/
	virtual void wheelEvent(QWheelEvent *poEvt); 
private:
	int min_, max_;
	QAction *numDisplay_;
	int realValue_;
	QString toolTipText_;
};
		
#endif /* NUMBERDISPLAY_H */ 
