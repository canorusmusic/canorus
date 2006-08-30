/** @file keysigpsp.cpp
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

#include "ui/keysigpsp.h"
#include "widgets/keyoffs.h"

typedef enum
{
	KS_CMajamin     =  7,
	KS_FMajdmin     =  6,
	KS_BMajgmin     =  5,
	KS_EflMajcmin   =  4,
	KS_AflMajfmin   =  3,
	KS_DflMajbflmin =  2,
	KS_GflMajeflmin =  1,
	KS_CflMajaflmin =  0,
	KS_GMajemin     =  8,
	KS_DMajbmin     =  9,
	KS_AMajfshmin   = 10,
	KS_EMajcshmin   = 11,
	KS_HMajgshmin   = 12,
	KS_FshMajdshmin = 13,
	KS_CshMajashmin = 14,
} KSig_T;
	
CAKeySigPSP::CAKeySigPSP( const QString & oTitle, QWidget * poParent /*  = 0 */,
	                      Qt::WFlags eFlags /* = 0 */ )
	: QDockWidget( oTitle, poParent, eFlags )
{
	QWidget *poDock = new QWidget( this );
	//Initialize widgets
	moKeySigWidget.setupUi( poDock );
	setWidget( poDock );
	// We have 15 key signatures by default
	moKeySigWidget.mpoKeySigSlider->setMaximum(14);
	moKeySigWidget.mpoKeySigListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	// Default: C Major, a minor
	moKeySigWidget.mpoKeySigSlider->setValue(7);
	// Funktionality for adding/removing new key signatures is
	// not needed at the moment, so hide the buttons
	moKeySigWidget.mpoKeySigButton->hide();
	moKeySigWidget.mpoRemoveButton->hide();
	// Setup map for slider-list updates
	moListSliderMap[KS_CMajamin]      = 0;
	moListSliderMap[KS_FMajdmin]      = 1;
	moListSliderMap[KS_BMajgmin]      = 2;
	moListSliderMap[KS_EflMajcmin]    = 3;
	moListSliderMap[KS_AflMajfmin]    = 4;
	moListSliderMap[KS_DflMajbflmin]  = 5;
	moListSliderMap[KS_GflMajeflmin]  = 6;
	moListSliderMap[KS_CflMajaflmin]  = 7;
	moListSliderMap[KS_GMajemin]      = 8;
	moListSliderMap[KS_DMajbmin]      = 9;
	moListSliderMap[KS_AMajfshmin]   = 10;
	moListSliderMap[KS_EMajcshmin]   = 11;
	moListSliderMap[KS_HMajgshmin]   = 12;
	moListSliderMap[KS_FshMajdshmin] = 13;
	moListSliderMap[KS_CshMajashmin] = 14;
	// Setup signal/slots for slider-list updates
	connect( moKeySigWidget.mpoKeySigSlider, SIGNAL( sliderMoved( int ) ),
	         this, SLOT( newKSSliderPos( int ) ) );
	connect( moKeySigWidget.mpoKeySigListWidget, SIGNAL( itemClicked( QTreeWidgetItem*, int ) ),
	         this, SLOT( newKSListViewItem( QTreeWidgetItem*, int ) ) );
	// Default: C Major, a minor
	moKeySigWidget.mpoKeySigListWidget->setCurrentItem(
	moKeySigWidget.mpoKeySigListWidget->topLevelItem( moListSliderMap[7] ) );
}

CAKeySigPSP::~CAKeySigPSP()
{
}

int CAKeySigPSP::getKeySignature()
{
	return moKeySigWidget.mpoKeySigSlider->value();
}

void CAKeySigPSP::newKSSliderPos( int iNewPos )
{
	// Select corresponding list item
	moKeySigWidget.mpoKeySigListWidget->setCurrentItem(
	moKeySigWidget.mpoKeySigListWidget->topLevelItem( moListSliderMap[iNewPos] ) );
}

void CAKeySigPSP::newKSListViewItem( QTreeWidgetItem *iNewItem, int)
{
	// Set corresponding slider position
	moKeySigWidget.mpoKeySigSlider->setValue( moListSliderMap.find( 
          moKeySigWidget.mpoKeySigListWidget->indexOfTopLevelItem( 
            iNewItem ) ).value() );
}
