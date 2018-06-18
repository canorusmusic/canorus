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
#include "export/canorusmlexport.h"
#include "import/canorusmlimport.h"
#include "canorus.h"
#include "core/settings.h"

/*!
	\class CAAutoRecovery
	\brief Class for making recovery files for application crashes

	Canorus creates recovery files in Canorus writeable settings directory for each currently
	opened document every number of minutes defined in CASettings.

	If the application is closed nicely, recovery files must be cleaned by calling
	cleanupRecovery() method.

	Otherwise Canorus looks for recovery files then next time it's executed and opens them
	automatically by calling openRecovery(). After recovering the files documents are marked
	as modified (so user needs to resave them, if closing the document by accident) and
	a special short-interval singleshot timer (see _saveAfterRecoveryInterval) is started
	to resave recovery files. This is usually needed when a user finds a bug, immediately
	repeats it and the autosave interval usually set to few minutes is not triggered yet.
	Recovery documents are not resaved immediately because Canorus might crash when recovering
	them (or soon after eg. mouse move) and is in unusable state until recovery files are
	manually deleted.

	Call saveRecovery() to save the currently opened documents to recovery files. The
	autosave timer's signal is connected to this slot.

	Settings class should already be initialized when creating instance of this class.
*/

/*!
	Initializes autosave. Reads the autosave timer settings from the CASettings class.
*/
CAAutoRecovery::CAAutoRecovery()
 : _saveAfterRecoveryTimer(0) {
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
	cleanupRecovery();

	QSet<CADocument*> documents;
	for (int i=0; i<CACanorus::mainWinList().size(); i++)
		documents << CACanorus::mainWinList()[i]->document();

	int c=0;
	for (QSet<CADocument*>::const_iterator i=documents.constBegin(); i!=documents.constEnd(); i++, c++) {
		CACanorusMLExport save;
		save.setStreamToFile( CASettings::defaultSettingsPath()+"/recovery"+QString::number(c) );
		save.exportDocument( *i );
		save.wait();
	}
}

/*!
	Deletes recovery files.
	This method is usually called when successfully quiting Canorus.
*/
void CAAutoRecovery::cleanupRecovery() {
	for ( int i=0; QFile::exists(CASettings::defaultSettingsPath()+"/recovery"+QString::number(i)); i++ ) {
		QString fileName = CASettings::defaultSettingsPath()+"/recovery"+QString::number(i);
		QFile::remove(fileName);
		if(QDir(fileName+" files").exists()) {
			foreach(QString entry, QDir(fileName+" files").entryList(QDir::Files)) {
				QFile::remove(fileName+" files/"+entry);
			}
			QDir().rmdir(fileName+" files");
		}
	}
}

/*!
	Searches for any not-cleaned up recovery files and opens them.
	Also shows the recovery message.
*/
void CAAutoRecovery::openRecovery() {
	QString documents;
	for ( int i=0; QFile::exists(CASettings::defaultSettingsPath()+"/recovery"+QString::number(i)); i++ ) {
		CACanorusMLImport open;
		open.setStreamFromFile( CASettings::defaultSettingsPath()+"/recovery"+QString::number(i) );
		open.importDocument();
		open.wait();
		if ( open.importedDocument() ) {
			open.importedDocument()->setModified(true); // warn that the file is unsaved, if closing
			open.importedDocument()->setFileName("");

			CAMainWin *mainWin = new CAMainWin();
			documents.append( tr("- Document %1 last modified on %2.").arg(open.importedDocument()->title()).arg(open.importedDocument()->dateLastModified().toString()) + "\n" );
			mainWin->openDocument( open.importedDocument() );
			mainWin->show();
		}
	}

	cleanupRecovery();

	if (!documents.isEmpty()) {
		if (_saveAfterRecoveryTimer ) delete _saveAfterRecoveryTimer;
		_saveAfterRecoveryTimer = new QTimer();
		_saveAfterRecoveryTimer->setInterval( 4000 );
		_saveAfterRecoveryTimer->setSingleShot( true );
		connect( _saveAfterRecoveryTimer, SIGNAL(timeout()), this, SLOT(saveRecovery()) );
		_saveAfterRecoveryTimer->start();

		QMessageBox::information(
				CACanorus::mainWinList()[ CACanorus::mainWinList().size()-1 ],
				tr("Document recovery"),
				tr("Previous session of Canorus was unexpectedly closed.\n\n\
The following documents were successfully recovered:\n%1").arg(documents)
		);
	}
}
