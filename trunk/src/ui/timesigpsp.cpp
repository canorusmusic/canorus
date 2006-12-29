/** @file timesigpsp.cpp
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

#include "ui/timesigpsp.h"

CATimeSigPSP::CATimeSigPSP( const QString & oTitle, QWidget * poParent /*  = 0 */,
	                    Qt::WFlags eFlags /* = 0 */ )
	: QDockWidget( oTitle, poParent, eFlags )
{
	QWidget *poDock = new QWidget( this );
	//Initialize widgets
	moTimeSigWidget.setupUi( poDock );
	setWidget( poDock );
	// We define both maximum number of beats and measure beat of 128
	moTimeSigWidget.mpoBeatsSpinBox->setMaximum( 128 );
	moTimeSigWidget.mpoBeatSpinBox->setMaximum( 128 );

	// Default: 4/4 (C Mensural)
	moTimeSigWidget.mpoBeatsSpinBox->setValue( 4 );
	moTimeSigWidget.mpoBeatSpinBox->setValue( 4 );

	// Funktionality for adding/removing new key signatures is
	// not needed at the moment, so hide the buttons
	moTimeSigWidget.mpoKeySigButton->hide();
	moTimeSigWidget.mpoRemoveButton->hide();

	// Setup signal/slots for spinbox updates
	connect( moTimeSigWidget.mpoBeatsSpinBox, SIGNAL( valueChanged( int ) ),
	         this, SLOT( on_mpoBeatsSpinBox_valueChanged( int ) ) );
	connect( moTimeSigWidget.mpoBeatSpinBox, SIGNAL( valueChanged( int ) ),
	         this, SLOT( on_mpoBeatSpinBox_valueChanged( int ) ) );
	// Default values
	miBeatNumber  = 4;
	miMeasureBeat = 4;
}

CATimeSigPSP::~CATimeSigPSP()
{
}

void CATimeSigPSP::getTimeSignature( int &iBeatNumber, int &iMeasureBeat )
{
	iBeatNumber  = miBeatNumber;
	iMeasureBeat = miMeasureBeat;
}

CAFixedTimeSig CATimeSigPSP::getTimeSignatureFixed()
{
	if( miBeatNumber == 4 && miMeasureBeat == 4 )
		return TS_44;
	if( miBeatNumber == 2 && miMeasureBeat == 2 )
		return TS_22;
	if( miBeatNumber == 3 && miMeasureBeat == 4 )
		return TS_34;
	if( miBeatNumber == 2 && miMeasureBeat == 4 )
		return TS_24;
	if( miBeatNumber == 3 && miMeasureBeat == 8 )
		return TS_38;
	if( miBeatNumber == 6 && miMeasureBeat == 8 )
		return TS_68;
	return TS_UNKNOWN;
}

void CATimeSigPSP::setTimeSignature( int iBeatNumber, int iMeasureBeat )
{
	moTimeSigWidget.mpoBeatsSpinBox->setValue( iBeatNumber );
	miBeatNumber = iBeatNumber;
	moTimeSigWidget.mpoBeatSpinBox->setValue( iMeasureBeat );
	miMeasureBeat = iMeasureBeat;
}

void CATimeSigPSP::on_mpoBeatsSpinBox_valueChanged( int iBeats )
{
	// Read value from Spinbox
	miBeatNumber = moTimeSigWidget.mpoBeatsSpinBox->value();
	// Time signature was changed -> allow to update score view
	emit timeSigChanged( miBeatNumber, miMeasureBeat );
}

void CATimeSigPSP::on_mpoBeatSpinBox_valueChanged( int iBeat )
{
	// Read value from Spinbox
	miMeasureBeat = moTimeSigWidget.mpoBeatSpinBox->value();
	// Time signature was changed -> allow to update score view
	emit timeSigChanged( miBeatNumber, miMeasureBeat );
}

