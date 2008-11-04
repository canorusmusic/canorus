/*!
        Copyright (c) 2006-2008, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.

        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

// Includes
#include <QDesktopServices>
#include <QUrl>
#include <QPainter>
#include <QPrinter>
#include <QSvgRenderer>
#include <QPrintDialog>

#include "ui/mainwin.h"
#include "export/svgexport.h"
#include "control/printctl.h"
#include "canorus.h"

CAPrintCtl::CAPrintCtl( CAMainWin *poMainWin )
{
	setObjectName("oPrintCtl");
	_poMainWin = poMainWin;
	_poSVGExport = new CASVGExport();
	if( poMainWin == 0 )
		qCritical("PrintCtl: No mainwindow instance available!");
	else
		 CACanorus::connectSlotsByName(_poMainWin, this);
	connect( _poSVGExport, SIGNAL( svgIsFinished( int ) ), this, SLOT( printSVG( int ) ) );
}

// Destructor
CAPrintCtl::~CAPrintCtl()
{
	if( _poSVGExport ) {
		delete _poSVGExport;
	}
	_poSVGExport = 0;
}

void CAPrintCtl::on_uiPrint_triggered()
{
	QDir oPath;
	QFile oTempFile( oPath.absolutePath ()+"/print.svg" );
	QString oTempFileName( oPath.absolutePath ()+"/print.svg" );
	if( !oTempFile.remove() )
	{
		qWarning("PrintCtl: Could not remove old print file %s, error %s", oTempFile.fileName().toAscii().constData(),
		oTempFile.errorString().toAscii().constData() );
		oTempFile.unsetError();
	}
	qDebug("PrintCtl: Print triggered via main window");
	// The exportDocument method defines the temporary file name and
	// directory, so we can only read it after the creation
	_poSVGExport->setStreamToFile( oTempFileName );
	_poSVGExport->exportDocument( _poMainWin->document() );
	_poSVGExport->wait();
	const QString roTempPath = _poSVGExport->getTempFilePath();
	// Copy the name for later output on the printer
	_oOutputSVGName = roTempPath;
}

void CAPrintCtl::printSVG( int iExitCode )
{
	// High resolution requires huge amount of memory :-(
	QPrinter oPrinter(QPrinter::ScreenResolution);
	QPainter oPainter;
	oPrinter.setFullPage(true);
	QPrintDialog oPrintDlg(&oPrinter);
	QDir oPath;
	QFile oTempFile( oPath.absolutePath ()+"/print.svg" );
	QSvgRenderer oRen;
	oTempFile.setFileName( _oOutputSVGName+".svg" );
	// Start by letting the user select the print settings
	if( oRen.load( oPath.absolutePath ()+"/print.svg" ) )
	{
		oPrintDlg.exec();
		// Actual printing with the help of the painter
		oPainter.begin( &oPrinter );
		// Draw the SVG image
		//oPainter.drawImage(0,0,poSVGPage->renderToImage(oPrinter.resolution(),oPrinter.resolution()));
		oPainter.setRenderHints( QPainter::Antialiasing );
		oRen.render( &oPainter );
		oPainter.end();
	}
}
