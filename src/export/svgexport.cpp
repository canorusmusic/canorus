/*!
        Copyright (c) 2008, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.

        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

// Includes
#include "export/lilypondexport.h"
#include "control/typesetctl.h"
#include "export/svgexport.h"

/*!
	\class CASVGExport
	\brief SVG export filter
	This class is used to export the document or parts of the document to SVG format.
	The most common use is to simply call the constructor
	\code
	CASVGExport( &textStream );
	\endcode

	\a textStream is usually the file stream or the content of the score source view widget.
*/

/*!
	Constructor for SVG export.
	Exports a document to LilyPond and create a SVG from it using the given text \a stream.
*/
CASVGExport::CASVGExport( QTextStream *stream )
 : CAExport(stream)
{
}

// Destructor
CASVGExport::~CASVGExport()
{
	if( _poTypesetCtl ) {
		delete _poTypesetCtl->getExporter();
		delete _poTypesetCtl;
	}
	_poTypesetCtl = 0;
}

void CASVGExport::startExport()
{
 	_poTypesetCtl = new CATypesetCtl();
	// For now we support only lilypond export
	_poTypesetCtl->setTypesetter( QString("lilypond") );
	_poTypesetCtl->setTSetOption("dbackend","svg",false,false);
	_poTypesetCtl->setExporter( new CALilyPondExport() );
	// Put lilypond output to console, could be shown on a canorus console later
	connect( _poTypesetCtl, SIGNAL( nextOutput( const QByteArray & ) ), this, SLOT( outputTypsetterOutput( const QByteArray & ) ) );
	connect( _poTypesetCtl, SIGNAL( typesetterFinished( int ) ), this, SLOT( svgFinished( int ) ) );
}

void CASVGExport::finishExport()
{
	if( _poTypesetCtl )
	{
		// Put lilypond output to console, could be shown on a canorus console later
		disconnect( _poTypesetCtl, SIGNAL( nextOutput( const QByteArray & ) ), this, SLOT( outputTypsetterOutput( const QByteArray & ) ) );
		disconnect( _poTypesetCtl, SIGNAL( typesetterFinished( int ) ), this, SLOT( svgFinished( int ) ) );
		delete _poTypesetCtl;
		_poTypesetCtl = 0; // Destruktor may not delete the same object again
	}
}

/*!
	Exports the document \a poDoc to LilyPond first and create a SVG from it
  using the Typesetter instance.
*/
void CASVGExport::exportDocumentImpl(CADocument *poDoc)
{
	if ( poDoc->sheetCount() < 1 ) {
		//TODO: no sheets, raise an error
		return;
	}
	// We cannot create the typesetter instance (a QProcess in the end)
	// in the constructor as it's parent would be in a different thread!
	startExport();
	// The exportDocument method defines the temporary file name and
	// directory, so we can only read it after the creation
	_poTypesetCtl->exportDocument( poDoc );
	const QString roTempPath = _poTypesetCtl->getTempFilePath();
	_poTypesetCtl->setTSetOption( QString("o"), roTempPath );
	_poTypesetCtl->setTSetOption("dbackend","svg",false,false);
	// Remove old svg file first, but ignore error (file might not exist)
	if( !file()->remove() )
	{
		qWarning("SVGExport: Could not remove old svg file %s, error %s", file()->fileName().toAscii().constData(),
				file()->errorString().toAscii().constData() );
		file()->unsetError();
	}
	_poTypesetCtl->runTypesetter(); // create svg
	// as we are not in the main thread wait until we are finished
	if( _poTypesetCtl->waitForFinished( -1 ) == false )
		qWarning("SVGExport: Typesetter %s was not finished","lilypond");
}

/*!
	Show the output \a roOutput of the typesetter on the console
*/
void CASVGExport::outputTypsetterOutput( const QByteArray &roOutput )
{
	// Output to error console
	qDebug( "%s", roOutput.data() );
}

/*!
	When the typesetter is finished copy the pdf file to the defined destination
*/
void CASVGExport::svgFinished( int iExitCode )
{
	QFile oTempFile( getTempFilePath()+".svg" );
  oTempFile.setFileName( getTempFilePath()+".svg" );
	qDebug("Exporting SVG file %s", file()->fileName().toAscii().data());
	if( !oTempFile.copy( file()->fileName() ) ) // Rename it, so we can delete the temporary file
	{
		qCritical("SVGExport: Could not copy temporary file %s, error %s", oTempFile.fileName().toAscii().constData(),
             oTempFile.errorString().toAscii().constData() );
		return;
	}
	emit svgIsFinished( iExitCode );
 	// Remove temporary files.
	if( !oTempFile.remove() )
	{
		qWarning("SVGExport: Could not remove temporary file %s, error %s", oTempFile.fileName().toAscii().constData(),
             oTempFile.errorString().toAscii().constData() );
		oTempFile.unsetError();
	}
	oTempFile.setFileName( getTempFilePath()+".ps" );
	// No warning as not every typesetter leaves postscript files behind
	oTempFile.remove();
	oTempFile.setFileName( getTempFilePath() );
	if( !oTempFile.remove() )
	{
		qWarning("SVGExport: Could not remove temporary file %s, error %s", oTempFile.fileName().constData(),
             oTempFile.errorString().toAscii().constData() );
		oTempFile.unsetError();
	}
	finishExport();
}

QString CASVGExport::getTempFilePath()
{
  return ( _poTypesetCtl ? _poTypesetCtl->getTempFilePath() : "" );
}

