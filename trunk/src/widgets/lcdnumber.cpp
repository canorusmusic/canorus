/** @file numberdisplay.cpp
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

#include <stdio.h>
#include <QToolTip>
#include <QMouseEvent>
#include <QAction>
#include "lcdnumber.h"

CALCDNumber::CALCDNumber (int iMin, int iMax, QWidget *poParent /* = 0 */ , 
                          QString oText /* = "" */) 
	: QLCDNumber(poParent) 
{
	 // Do not allow  a larger minimum than maximum
	 if (iMin > iMax) iMin = iMax;
	 min_ = iMin; max_ = iMax;
	 setRealValue( 0 ); // 0 = All!
	 //setMaximumSize (20, 60);
	 setToolTip( oText );
}

void CALCDNumber::setRealValue(int iVal) {
	if ( (iVal <= max_) && (iVal >= 0) ) realValue_ = iVal;
	// 0 = All => Display letter "A" from Hex
	if( realValue_ == 0 )
	{	 
		QLCDNumber::setMode( QLCDNumber::Hex );
		display( 10 );
	}
	else
	{	 
		QLCDNumber::setMode( QLCDNumber::Dec );
		display( realValue_ );
	}

	emit valChanged(iVal);
}

int CALCDNumber::getRealValue() {
	return realValue_;
}
			
void CALCDNumber::setMin(int iMin) {
	// Do not allow  a larger minimum than maximum
	if (iMin > max_) return;
	min_ = iMin;
	if (min_ > getRealValue()) {
		setRealValue(min_);
	}
}

void CALCDNumber::setMax(int iMax) {
	// Do not allow  a larger minimum than maximum
	if (iMax < min_) return;
	max_ = iMax;
	if (max_ < getRealValue()) {
		setRealValue(max_);
	}
}

bool CALCDNumber::isZero() 
{
	return getRealValue() == 0;
}

void CALCDNumber::mousePressEvent(QMouseEvent *poEvt) {
	int iNewVal;
	switch (poEvt->button()) 
	{
		case Qt::LeftButton: iNewVal = getRealValue() + 1;
				   if (iNewVal > max_) break;
				   setRealValue(iNewVal);
				   break;

		default:	   iNewVal = getRealValue() - 1;
				   if (iNewVal < min_) break;
				   setRealValue(iNewVal);
				   break;
	}
}

void CALCDNumber::wheelEvent(QWheelEvent *poEvt) {
	int iNewVal;
	if (poEvt->delta() < 0) {
		iNewVal = getRealValue() + 1;
		if (iNewVal > max_) return;
		
		setRealValue(iNewVal);
	} else if (poEvt->delta() > 0) {
		iNewVal = getRealValue() - 1;
		if (iNewVal < min_) return;
		
		setRealValue(iNewVal);
	}
}
