/*!
        Copyright (c) 2006-2010, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.

        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

// Includes
#include <QMessageBox>

#include "ui/mainwin.h"
#include "ui/dummyui.h"
#include "ctl/dummyctl.h"
#include "ui/dummyuiobj.h" // Does not exist, this is only a dummy example!
#include "canorus.h"

/*! 
	\class CADummyUI
	\brief Dummy example user interface objects creation
	
	Use this dummy class for creating your own user interface
	object creation classes
*/

/*!
	Default Construktor
	Besides standard initialization it creates a dummy ui object.
*/
CADummyUI::CADummyUI( CAMainWin *poMainWin )
{
	setObjectName("oDummyUI");
	_poMainWin = poMainWin;
	_poDummyCtl = new CADummyCtl( poMainWin );
	_poDummyUIObj = new DummyUIObj( poMainWin );
	if( poMainWin == 0 )
		qCritical("DummyCtl: No mainwindow instance available!");
}

// Destructor
CADummyUI::~CADummyUI()
{
	if( _poDummyUIObj ) {
		delete _poDummyUIObj;
	}
	_poDummyUIObj = 0;
}

CADummyUI::updateDummyUIObjs()
{
	_poDummyUIObj->updateDummyUI();
}

