/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/file.h"
#include <QTextStream>
#include <QFile>

/*!
	\class CAFile
	\brief File import/export base class
	
	This class brings tools for manipulating with files and streams (most notably import and export).
	Classes CAImport and CAExport inherit this class and implement specific methods for import and export.
	
	All file operations are done in a separate thread. While the file operations are in progress user
	can poll the status by calling status(), progress() and readableStatus() for human-readable status
	defined by the filter. Waiting for the thread to be finished can be implemented by an endless busy-wait
	isRunning() function or by catching one of the signals emitted by children import and export classes.
	
	\sa CAImport, CAExport
*/

CAFile::CAFile() : QThread() {
	setProgress( 0 );
	setStatus( 0 );
	setStream( 0 );
	setFile( 0 );
}

/*!
	Destructor.
	Also destroys the created stream and file, if set.
*/
CAFile::~CAFile() {
	if ( file() ) {
		delete stream();
		delete file();
	}
}

/*!
	Creates and sets the stream from the file named \a filename.
	Stream is Read-only.
	This method is usually called from the main window when opening a document.
	This method is also very important for python developers as they cannot directly
	access QTextStream class, so they call this wrapper instead with a simple string as parameter.
*/
void CAFile::setStreamFromFile( const QString filename ) {
	setFile( new QFile( filename ) );
	file()->open( QIODevice::ReadOnly );
	setStream( new QTextStream(file()) );
}

/*!
	Creates and sets the stream from the file named \a filename.
	Stream is Read-Write.
	This method is usually called from the main window when saving the document.
	This method is also very important for python developers as they cannot directly
	access QTextStream class, so they call this wrapper instead with a simple string as parameter.
*/
void CAFile::setStreamToFile( const QString filename ) {
	setFile( new QFile( filename ) );
	file()->open( QIODevice::ReadWrite );
	setStream( new QTextStream(file()) );
}

/*!
	\function int CAFile::status()
	
	Default:
	  0 - filter is ready
	  1 - filter is busy
	 -1 - error during operations
*/

/*!
	\function const QString CAFile::readableStatus()
	
	Human readable status eg. the one shown in the status bar.
	The current status should be reimplemented in children classes.
*/
