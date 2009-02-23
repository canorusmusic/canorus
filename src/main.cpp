/*!
	Copyright (c) 2006-2009, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QApplication>
#include <QSplashScreen>
#include <QFont>

// Python.h needs to be loaded first!
#include "canorus.h"
#include "ui/mainwin.h"
#include "ui/settingsdialog.h"
#include "interface/pluginmanager.h"

#include <iostream>

#ifdef Q_WS_X11
#include <signal.h>
void catch_sig(int)
{
	qApp->quit();
}
#endif

/*!
	Main function. This is the first function called when Canorus is run.
	It initializes CACanorus class and creates the main window.
*/
int main(int argc, char *argv[]) {
	QApplication mainApp(argc, argv);

#ifdef Q_WS_X11
	signal(SIGINT, catch_sig);
	signal(SIGQUIT, catch_sig);
#endif

	CACanorus::initSearchPaths();

	QPixmap splashPixmap( 400, 300 );
	splashPixmap = QPixmap("images:splash.png");

	QSplashScreen splash( splashPixmap );
	QFont font("Century Schoolbook L");
	font.setPixelSize(17);
	splash.setFont(font);
	mainApp.processEvents();

	// Set main application properties
	CACanorus::initMain();

	// Parse switch and settings command line arguments
	if ( !CACanorus::parseSettingsArguments(argc, argv) )
		return 0;

	splash.show();

	// Load system translation if found
	CACanorus::initTranslations();

	// Init MIDI devices
	CACanorus::initPlayback();

	// Load config file
	CASettingsDialog::CASettingsPage showSettingsPage = CACanorus::initSettings();

	// Init dialogs etc.
	CACanorus::initCommonGUI();

	// Enable scripting and plugins subsystem
	splash.showMessage( QObject::tr("Initializing Scripting engine", "splashScreen"), Qt::AlignBottom|Qt::AlignLeft, Qt::white );
	mainApp.processEvents();
	CACanorus::initScripting();

	// Finds all the plugins
	splash.showMessage( QObject::tr("Reading Plugins", "splashScreen"), Qt::AlignBottom|Qt::AlignLeft, Qt::white );
	mainApp.processEvents();
	CAPluginManager::readPlugins();

	// Initialize autosave
	splash.showMessage( QObject::tr("Initializing Automatic recovery", "splashScreen"), Qt::AlignBottom|Qt::AlignLeft, Qt::white );
	mainApp.processEvents();
	CACanorus::initAutoRecovery();

	// Initialize help
	splash.showMessage( QObject::tr("Initializing Help", "splashScreen"), Qt::AlignBottom|Qt::AlignLeft, Qt::white );
	mainApp.processEvents();
	CACanorus::initHelp();

	// Initialize undo/redo stacks
	splash.showMessage( QObject::tr("Initializing Undo/Redo framework", "splashScreen"), Qt::AlignBottom|Qt::AlignLeft, Qt::white );
	mainApp.processEvents();
	CACanorus::initUndo();

	// Load bundled fonts
	splash.showMessage( QObject::tr("Loading fonts", "splashScreen"), Qt::AlignBottom|Qt::AlignLeft, Qt::white );
	mainApp.processEvents();
	CACanorus::initFonts();

	// Check for any crashed Canorus sessions and open the recovery files
	splash.showMessage( QObject::tr("Searching for recovery documents", "splashScreen"), Qt::AlignBottom|Qt::AlignLeft, Qt::white );
	mainApp.processEvents();
	CACanorus::autoRecovery()->openRecovery();

	// Creates a main window of a document to open if passed in command line
	splash.showMessage( QObject::tr("Initializing Main window", "splashScreen"), Qt::AlignBottom|Qt::AlignLeft, Qt::white );
	mainApp.processEvents();
	CACanorus::parseOpenFileArguments(argc, argv);

	// If no file to open is passed in command line, create a new default main window. It's shown automatically by CACanorus::addMainWin().
	if (!CACanorus::mainWinCount()) {
		CAMainWin *mainWin = new CAMainWin();
		mainWin->newDocument();
		mainWin->show();
	}
	splash.close();

	// Show settings dialog, if needed (eg. MIDI setup when running Canorus for the first time)
	if ( showSettingsPage != CASettingsDialog::UndefinedSettings ) {
		CASettingsDialog( showSettingsPage, CACanorus::mainWinAt(0) );
	}

	return mainApp.exec();
}
