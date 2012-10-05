/*!
        Copyright (c) 2006-2010, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.

        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

// Includes
#include <QMessageBox>

#include "ui/mainwin.h"
#include "control/dummyctl.h"
#include "dummy/dummy.h" // Does not exist, this is only a dummy example!
#include "canorus.h"

/*! 
	\class CADummyCtl
	\brief Dummy example ctl for user interface actions
	
	Use this dummy class for creating your own interface
	action classes. This Ctl was based on CAPreviewCtl originally
	one of the two first control classes in Canorus.
*/

/*!
	Default Construktor
	Besides standard initialization it creates a dummy object.
	Though not implemented we assume that the dummy object has at
	least one signal called dummyToggle.
	We connect this signal manually to our slot myToggle.
*/
CADummyCtl::CADummyCtl( CAMainWin *poMainWin )
{
	setObjectName("oDummyCtl");
	_poMainWin = poMainWin;
	_poDummy = new Dummy();
	if( poMainWin == 0 )
		qCritical("DummyCtl: No mainwindow instance available!");
	else
		 CACanorus::connectSlotsByName(_poMainWin, this);
	connect( _poDummy, SIGNAL( dummyToggle( int ) ), this, SLOT( myToggle( int ) ) );
}

// Destructor
CADummyCtl::~CADummyCtl()
{
	if( _poDummy ) {
		delete _poDummy;
	}
	_poDummy = 0;
}

void CADummyCtl::on_uiDummy_triggered()
{
	// Do something
}

void CADummyCtl::myToggle( int iOn )
{
	// Do something else
}

