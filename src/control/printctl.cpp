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
#include <QPrintDialog>
// For some unknown reason PATH_SUFFIXES does not work
// in the CMakeList.txt configure script (it does return
// the wrong path, f.e. /usr/include under Linux)
#include <poppler/qt4/poppler-qt4.h>

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
	connect(  _poTypesetCtl, SIGNAL( typesetterFinished( int ) ), this, SLOT( printPDF( int ) ) );
}

// Destructor
CAPrintCtl::~CAPrintCtl()
{
	if( _poTypesetCtl ) {
		delete _poTypesetCtl->getExporter();
		delete _poTypesetCtl;
	}
	_poTypesetCtl = 0;
}

void CAPrintCtl::on_uiPrint_triggered()
{
	qDebug("PrintCtl: Print triggered via main window");
	// The exportDocument method defines the temporary file name and
	// directory, so we can only read it after the creation
	_poTypesetCtl->exportDocument( _poMainWin->document() );
	const QString roTempPath = _poTypesetCtl->getTempFilePath();
	_poTypesetCtl->setTSetOption( QString("o"), roTempPath );
	_poTypesetCtl->runTypesetter();
	// Copy the name for later output on the printer
	_oOutputPDFName = roTempPath;
}
 
void CAPrintCtl::outputTypsetterOutput( const QByteArray &roOutput )
{
	// Output to error console
	qDebug( "%s", roOutput.data() );
}

void CAPrintCtl::printPDF( int iExitCode )
{
	QPrinter oPrinter(QPrinter::PrinterResolution);
	QPainter oPainter;
	oPrinter.setFullPage(true);
	QPrintDialog oPrintDlg(&oPrinter);
	QDir oPath;
	QFile oTempFile( oPath.absolutePath ()+"/print.pdf" );
	if( !oTempFile.remove() )
  {
		qWarning("PreviewCtl: Could not remove old print file %s, error %s", oTempFile.fileName().toAscii().constData(),
             oTempFile.errorString().toAscii().constData() );
    oTempFile.unsetError();
  }
  oTempFile.setFileName( _oOutputPDFName+".pdf" );
	// Start by letting the user select the print settings
	qDebug("Printing PDF file %s", QString("file://"+oPath.absolutePath ()+"/print.pdf").toAscii().data());
	if( !oTempFile.copy( oPath.absolutePath ()+"/preview.pdf" ) ) // Rename it, so we can delete the temporary file
  {
		qCritical("PreviewCtl: Could not copy temporary file %s, error %s", oTempFile.fileName().toAscii().constData(),
             oTempFile.errorString().toAscii().constData() );
    return;
  }
	// Read PDF document from disk to draw it's individual pages
	Poppler::Document *poPDFDoc = Poppler::Document::load(QString( oPath.absolutePath ()+"print.pdf"));
	// Set the maximum number of pages
	if( poPDFDoc )
	{
		int iNumPages = poPDFDoc->numPages(), iC;
		oPrintDlg.setMinMax( 1, iNumPages );
		oPrintDlg.exec();
		// Get the selected number of pages
		iNumPages = oPrinter.toPage() - oPrinter.fromPage();
		Poppler::Page *poPDFPage;
		// Actual printing with the help of the painter
		oPainter.begin( &oPrinter );
		// @ToDo: Limit to the page range the user wants to print
		// Read the current page
		for( iC= oPrinter.fromPage(); iC< oPrinter.toPage(); ++iC )
		{
			if( oPrinter.pageOrder() == QPrinter::FirstPageFirst )
				poPDFPage = poPDFDoc->page(iC);
			else
				poPDFPage = poPDFDoc->page(iNumPages-iC);
			if( poPDFPage )
			{
				oPainter.drawImage(0,0,poPDFPage->renderToImage(oPrinter.resolution(),oPrinter.resolution()));
				oPrinter.newPage();
			}
		}
		// Last page (separated to avoid extra if in for loop for last page)
		if( oPrinter.pageOrder() == QPrinter::FirstPageFirst )
			poPDFPage = poPDFDoc->page(iC);
		else
			poPDFPage = poPDFDoc->page(iNumPages-iC);
		if( poPDFPage )
			oPainter.drawImage(0,0,poPDFPage->renderToImage(oPrinter.resolution(),oPrinter.resolution()));
		oPainter.end();
  }
  // Remove temporary files. 
	if( !oTempFile.remove() )
  {
		qWarning("PreviewCtl: Could not remove temporary file %s, error %s", oTempFile.fileName().toAscii().constData(),
             oTempFile.errorString().toAscii().constData() );
    oTempFile.unsetError();
  }
	oTempFile.setFileName( _oOutputPDFName+".ps" );
	// No warning as not every typesetter leaves postscript files behind
	oTempFile.remove();
	oTempFile.setFileName( _oOutputPDFName );
	if( !oTempFile.remove() )
  {
		qWarning("PreviewCtl: Could not remove temporary file %s, error %s", oTempFile.fileName().constData(),
             oTempFile.errorString().toAscii().constData() );
    oTempFile.unsetError();
  }
}

