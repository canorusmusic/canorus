/*! 
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "core/autorecovery.h"

#include <QSet>
#include <QTimer>
#include <QFile>
#include <QMessageBox>
#include "core/canorusml.h"
#include "core/canorus.h"
#include "core/settings.h"

/*!
	\class CAAutoRecovery
	\brief Class for making recovery files for application crashes
	
	Canorus creates recovery files in Canorus writeable settings directory for each currently
	opened document every number minutes defined in CASettings.
	If the application is closed nicely, recovery files are deleted as well by calling
	cleanupRecovery() method.
	Otherwise Canorus looks for recovery files everytime it's executed and opens them
	automatically by calling openRecovery().
	Call saveRecovery() to save the currently opened documents to recovery files. The
	auto recovery timer's signal is also connected to this slot.
	
	Settings class should already be initialized when creating instance of this class.
*/

/*!
	Initializes autosave. Reads the autosave timer settings from the CASettings class.
*/
CAAutoRecovery::CAAutoRecovery() {
	_autoRecoveryTimer = new QTimer(this);
	_autoRecoveryTimer->setSingleShot( false );
	connect( _autoRecoveryTimer, SIGNAL(timeout()), this, SLOT(saveRecovery()) );
	updateTimer(); // reads interval from settings and starts the timer
}

CAAutoRecovery::~CAAutoRecovery() {
	delete _autoRecoveryTimer;
}

/*!
	Sets auto recovery interval to \a msec miliseconds.
*/
void CAAutoRecovery::updateTimer() {
	_autoRecoveryTimer->stop();
	_autoRecoveryTimer->setInterval( CACanorus::settings()->autoRecoveryInterval()*60000 );
	if ( CACanorus::settings()->autoRecoveryInterval() )
		_autoRecoveryTimer->start();
}

/*!
	Saves the currently opened documents into settings folder named recovery0, recovery1 etc.
*/
void CAAutoRecovery::saveRecovery() {
	QSet<CADocument*> documents;
	for (int i=0; i<CACanorus::mainWinCount(); i++)
		documents << CACanorus::mainWinAt(i)->document();
	
	int c=0;
	for (QSet<CADocument*>::const_iterator i=documents.constBegin(); i!=documents.constEnd(); i++, c++) {
		CACanorusML::saveDocumentToFile( *i, CACanorus::settingsPath()+"/recovery"+QString::number(c) );
	}
	
	while (	QFile::exists(CACanorus::settingsPath()+"/recovery"+QString::number(c)) ) {
		QFile::remove(CACanorus::settingsPath()+"/recovery"+QString::number(c));
		c++;
	}
}

/*!
	Deletes recovery files.
	This method is usually called when successfully quiting Canorus.
*/
void CAAutoRecovery::cleanupRecovery() {
	for ( int i=0; QFile::exists(CACanorus::settingsPath()+"/recovery"+QString::number(i)); i++ ) {
		QFile::remove(CACanorus::settingsPath()+"/recovery"+QString::number(i));
	}
}

/*!
	Searches for any not-cleaned up recovery files and opens them.
	Also shows the recovery message.
*/
void CAAutoRecovery::openRecovery() {
	QString documents;
	for ( int i=0; QFile::exists(CACanorus::settingsPath()+"/recovery"+QString::number(i)); i++, documents.append("\n") ) {
		CAMainWin *mainWin = new CAMainWin();
		CACanorus::addMainWin( mainWin );
		CADocument *document = mainWin->openDocument( CACanorus::settingsPath()+"/recovery"+QString::number(i) );
		documents.append( tr("- Document %1 last modified on %2.").arg(document->title()).arg(document->dateLastModified().toString()) );
	}
	
	cleanupRecovery();
	
	if (!documents.isEmpty()) {
		QMessageBox::information(
				CACanorus::mainWinAt( CACanorus::mainWinCount()-1 ),
				tr("Document recovery"),
				tr("Previous session of Canorus was unexpectedly closed.\n\n\
The following documents were successfully recovered:\n%1").arg(documents)
		);
	}
}
