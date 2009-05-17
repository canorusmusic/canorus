/*!
        Copyright (c) 2006-2008, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.

        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

// Includes
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QProcess> // needed for custom pdf viewer application

#include "canorus.h" // needed for settings
#include "core/settings.h"
#include "ui/mainwin.h"
#include "export/pdfexport.h"
#include "control/previewctl.h"
#include "canorus.h"

CAPreviewCtl::CAPreviewCtl( CAMainWin *poMainWin )
{
	setObjectName("oPreviewCtl");
	_poMainWin = poMainWin;
	_poPDFExport = new CAPDFExport();
	if( poMainWin == 0 )
		qCritical("PreviewCtl: No mainwindow instance available!");
	else
		 CACanorus::connectSlotsByName(_poMainWin, this);
	connect( _poPDFExport, SIGNAL( pdfIsFinished( int ) ), this, SLOT( showPDF( int ) ) );
}

// Destructor
CAPreviewCtl::~CAPreviewCtl()
{
	if( _poPDFExport ) {
		delete _poPDFExport;
	}
	_poPDFExport = 0;
}

void CAPreviewCtl::on_uiPrintPreview_triggered()
{
	QDir oPath( QDir::tempPath() );
	QFile oTempFile( oPath.absolutePath ()+"/preview.pdf" );
	QString oTempFileName( oPath.absolutePath()+"/preview.pdf" );
	if( !oTempFile.remove() )
 	{
		qWarning("PreviewCtl: Could not remove old preview file %s, error %s", oTempFile.fileName().toAscii().constData(),
             oTempFile.errorString().toAscii().constData() );
		oTempFile.unsetError();
 	}
	qDebug("PreviewCtl: Preview triggered via main window");
	// The exportDocument method defines the temporary file name and
	// directory, so we can only read it after the creation
	_poPDFExport->setStreamToFile( oTempFileName );
	//_poPDFExport->exportDocument( _poMainWin->document() );
	_poPDFExport->exportSheet( _poMainWin->currentSheet() );
	_poPDFExport->wait();
	const QString roTempPath = _poPDFExport->getTempFilePath();
	// Copy the name for later output on the printer
	_oOutputPDFName = roTempPath;
}

void CAPreviewCtl::showPDF( int iExitCode )
{
	if ( iExitCode ) {
		QMessageBox::critical( _poMainWin, tr("Error running preview"), tr("Error while running the typesetter.\n\nPlease install LilyPond (visit http://www.lilypond.org) and check the settings.") );
		return;
	}

	bool success;
	// Show the pdf file with the default pdf system viewer
	if ( CACanorus::settings()->useSystemDefaultPdfViewer() ) {
		success = QDesktopServices::openUrl( QUrl( QString("file:")+QDir::tempPath()+"/preview.pdf" ) );
	} else { // Use viewer specified in settings (location and command) to show the pdf file
		success = QProcess::startDetached( CACanorus::settings()->pdfViewerLocation(), QStringList() << QDir::tempPath()+"/preview.pdf" );
	}

	if ( !success ) {
		QMessageBox::critical( _poMainWin, tr("Error running preview"), tr("Unable to show %1.\n\nPlease install a PDF viewer and check the settings.").arg(QDir::tempPath()+"/preview.pdf") );
	}
}
