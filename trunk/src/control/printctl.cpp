/*!
        Copyright (c) 2006-2008, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.
        
        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

// Includes
#include "ui/mainwin.h"
#include "export/lilypondexport.h"
#include "control/typesetctl.h"
#include "control/printctl.h"
#include "canorus.h"

CAPrintCtl::CAPrintCtl( CAMainWin *poMainWin )
{
	setObjectName("oPrintCtl");
	_poMainWin = poMainWin;
 	_poTypesetCtl = new CATypesetCtl();
	// For now we support only lilypond export
	_poTypesetCtl->setTypesetter( QString("lilypond") );
	_poTypesetCtl->setExporter( new CALilyPondExport() );
	// Put lilypond output to console, could be shown on a canorus console later
	connect( _poTypesetCtl, SIGNAL( nextOutput( const QByteArray & ) ), this, SLOT( outputTypsetterOutput( const QByteArray & ) ) );
	if( poMainWin == 0 )
		qCritical("PrintCtl: No mainwindow instance available!");
	else
		 CACanorus::connectSlotsByName(_poMainWin, this);
}

// Destructor
CAPrintCtl::~CAPrintCtl()
{
	if( _poTypesetCtl )
		delete _poTypesetCtl;
	_poTypesetCtl = 0;
}

void CAPrintCtl::on_uiPrint_triggered()
{
	qDebug("PrintCtl: Print triggered via main window");
	_poTypesetCtl->exportDocument( _poMainWin->document() );
	_poTypesetCtl->runTypesetter();
}
 
void CAPrintCtl::outputTypsetterOutput( const QByteArray &roOutput )
{
	// Output to error console
	qDebug( "%s", roOutput.data() );
}
