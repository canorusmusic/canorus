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
	defined by the filter. Waiting for the thread to be finished can be implemented by calling QThread::wait()
	or by catching the signals emitted by children import and export classes.

	\sa CAImport, CAExport
*/

CAFile::CAFile() : QThread() {
	setProgress( 0 );
	setStatus( 0 );
	setStream( 0 );
	setFile( 0 );
	_deleteStream = false;
}

/*!
	Destructor.
	Also destroys the created stream and file, if set.
*/
CAFile::~CAFile() {
	if( stream() && _deleteStream )
		delete stream();
	if ( file() )
		delete file();
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

	if ( file()->open( QIODevice::ReadOnly ) )
	{
		if(stream() && _deleteStream) {
			delete stream();
		}
		setStream( new QTextStream(file()) );
		_deleteStream = true;
	}
}

/*!
	Creates and sets the stream from the file named \a filename.
	Stream is Read-Write.
	This method is usually called from the main window when saving the document.
	This method is also very important for python developers as they cannot directly
	access QTextStream class, so they call this wrapper instead with a simple string as parameter.
*/
void CAFile::setStreamToFile( const QString filename ) {
	if(stream() && _deleteStream)
		delete stream();
	setFile( new QFile( filename ) );

	if ( file()->open( QIODevice::WriteOnly ) ) {
		if(stream() && _deleteStream) {
			delete stream();
		}
		setStream( new QTextStream(file()) );
		_deleteStream = true;
	}
}

/*!
	Creates and sets the stream from the given device.
	Read-write if the given device is not already open.
*/
void CAFile::setStreamToDevice(QIODevice* device)
{
	if(stream() && _deleteStream)
		delete stream();
	if(!device->isOpen())
		device->open(QIODevice::ReadWrite);
	setStream(new QTextStream(device));
	_deleteStream = true;
}

/**
	Creates and sets the stream from the given device.
	Read-only if the device is not already open.
*/
void CAFile::setStreamFromDevice(QIODevice* device)
{
	if(stream() && _deleteStream)
		delete stream();
	if(!device->isOpen())
		device->open(QIODevice::ReadOnly);
	setStream(new QTextStream(device));
	_deleteStream = true;
}

/*!
	\function int CAFile::status()

	The number describes the current status of the operations.

	Possible values are:
	  0              - filter is ready (not started yet or successfully done)
	  greater than 0 - filter is busy, custom filter status
	  -1             - file not found or cannot be opened
	  lesser than -1 - custom filter errors
*/

/*!
	\function const QString CAFile::readableStatus()

	Human readable status eg. the one shown in the status bar.
	The current status should be reimplemented in children classes.
*/
