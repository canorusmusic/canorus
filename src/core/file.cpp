/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/file.h"
#include <QTextStream>
#include <QFile>

CAFile::CAFile() : QThread() {
	setProgress( 0 );
	setStatus( 0 );
	setStream( 0 );
	setFile( 0 );
}

CAFile::~CAFile() {
	if ( file() ) {
		delete stream();
		delete file();
	}
}

void CAFile::setStreamFromFile( const QString filename ) {
	setFile( new QFile( filename ) );
	file()->open( QIODevice::ReadOnly );
	setStream( new QTextStream(file()) );
}
