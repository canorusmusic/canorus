/*!
        Copyright (c) 2006-2008, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.
        
        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

// Includes
#include <QDesktopServices>
#include <QUrl>

#include "ui/mainwin.h"
#include "export/lilypondexport.h"
#include "control/typesetctl.h"
#include "control/previewctl.h"
#include "canorus.h"

CAPreviewCtl::CAPreviewCtl( CAMainWin *poMainWin )
{
	setObjectName("oPreviewCtl");
	_poMainWin = poMainWin;
 	_poTypesetCtl = new CATypesetCtl();
	// For now we support only lilypond export
	_poTypesetCtl->setTypesetter( QString("lilypond") );
	_poTypesetCtl->setExporter( new CALilyPondExport() );
	// Put lilypond output to console, could be shown on a canorus console later
	connect( _poTypesetCtl, SIGNAL( nextOutput( const QByteArray & ) ), this, SLOT( outputTypsetterOutput( const QByteArray & ) ) );
	if( poMainWin == 0 )
		qCritical("PreviewCtl: No mainwindow instance available!");
	else
		 CACanorus::connectSlotsByName(_poMainWin, this);
	connect(  _poTypesetCtl, SIGNAL( typesetterFinished( int ) ), this, SLOT( showPDF( int ) ) );
}

// Destructor
CAPreviewCtl::~CAPreviewCtl()
{
	if( _poTypesetCtl ) {
		delete _poTypesetCtl->getExporter();
		delete _poTypesetCtl;
	}
	_poTypesetCtl = 0;
}

void CAPreviewCtl::on_uiPrintPreview_triggered()
{
	qDebug("PreviewCtl: Preview triggered via main window");
	// The exportDocument method defines the temporary file name and
	// directory, so we can only read it after the creation
	_poTypesetCtl->exportDocument( _poMainWin->document() );
	const QString roTempPath = _poTypesetCtl->getTempFilePath();
	_poTypesetCtl->setTSetOption( QString("o"), roTempPath );
	_poTypesetCtl->runTypesetter();
	// Copy the name for later output on the printer
	_oOutputPDFName = roTempPath;
}
 
void CAPreviewCtl::outputTypsetterOutput( const QByteArray &roOutput )
{
	// Output to error console
	qDebug( "%s", roOutput.data() );
}

void CAPreviewCtl::showPDF( int iExitCode )
{
	qDebug("Showing PDF file %s", QString("file://"+_oOutputPDFName+".pdf").toAscii().data());
	// First version show the pdf file with the default pdf system viewer
	QDesktopServices::openUrl( QUrl( QString("file://")+_oOutputPDFName+".pdf" ) );
  // Remove temporary files. Question: When ? QDesktopServices::openURL immediately returns here
	QFile oTempFile( _oOutputPDFName+".pdf" );
	if( !oTempFile.remove() )
		qWarning("PreviewCtl: Could not remove temporary file %s", QString(_oOutputPDFName+".pdf").toAscii().constData() );
	oTempFile.setFileName( _oOutputPDFName+".ps" );
	// No warning as not every typesetter leaves postscript files behind
	oTempFile.remove();
	oTempFile.setFileName( _oOutputPDFName );
	if( !oTempFile.remove() )
		qWarning("PreviewCtl: Could not remove temporary file %s",_oOutputPDFName.toAscii().constData() );
}

