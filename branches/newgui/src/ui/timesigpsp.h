/** @file timesigpsp.h
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

#ifndef TIMESIGPSP_H_
#define TIMESIGPSP_H_

#include <QDockWidget>

#include "ui_timesig.h"

class QTreeWidgetItem;

// Fixed time signature values (Canorus toolbar)
enum CAFixedTimeSig {
	TS_44,
	TS_22,
	TS_34,
	TS_24,
	TS_38,
	TS_68,
	TS_UNKNOWN
};

// Key signature perspective
class CATimeSigPSP : public QDockWidget
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
	CATimeSigPSP( const QString & oTitle, QWidget * poParent = 0, Qt::WFlags eFlags = 0 );
	virtual ~CATimeSigPSP();
	
	/**
	* Returns the currently selected time signature as integer values
	* 
	*/
	void getTimeSignature( int &iBeatNumber, int &iMeasureBeat );
	/**
	* Returns the currently selected time signature as fixed value
	* 
	*/
	CAFixedTimeSig getTimeSignatureFixed();
	/**
	* Sets the time signature as integer values
	* 
	*/
	void setTimeSignature( int iBeatNumber, int iMeasureBeat );

signals:
        void timeSigChanged( int iBeats, int iBeat );
	
protected slots:
	/**
	* Updates the number of beats
	* 
	* @param  iBeats      New number of beats for a measure
	*
	*/
	void on_mpoBeatsSpinBox_valueChanged( int iBeats );
	/**
	* Updates the number of beats
	* 
	* @param  iBeat       measure beat
	*
	*/
	void on_mpoBeatSpinBox_valueChanged( int iBeat );
	
protected:
	Ui::TimeSig moTimeSigWidget;  /// Time signature perspective represantative
        int         miBeatNumber;     /// Currently selected number of beats
        int         miMeasureBeat;    /// Currently selected measure beat
};

#endif /*TIMESIGPSP_H_*/
