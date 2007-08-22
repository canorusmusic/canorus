/*! 
	Copyright (c) 2006-2007, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QApplication>
#include <QTranslator>
#include <QLocale>

// Python.h needs to be loaded first!
#include "core/canorus.h"
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
	
	// Set main application properties
	CACanorus::initMain();
	
	// Parse switch and settings command line arguments
	CACanorus::parseSettingsArguments(argc, argv);
	
	// Load config file
	CASettingsDialog::CASettingsPage showSettingsPage = 
		CACanorus::initSettings();
	
	// Init dialogs etc.
	CACanorus::initCommonGUI();
	
	// Load system translation if found
	QList<QString> translationLocations =
		CACanorus::locateResource(QString("lang/") + QLocale::system().name() + ".qm");
	QTranslator translator;
	if (translationLocations.size()) {
		translator.load(translationLocations[0]);
		mainApp.installTranslator(&translator);
	}
	
	// Enable scripting and plugins subsystem
	CACanorus::initScripting();
	
	// Finds all the plugins
	CAPluginManager::readPlugins();
	
	// Initialize autosave
	CACanorus::initAutoRecovery();
	
	// Initialize undo/redo stacks
	CACanorus::initUndo();
	
	// Check for any crashed Canorus sessions and open the recovery files
	CACanorus::autoRecovery()->openRecovery();
	
	// Creates a main window of a document to open if passed in command line
	CACanorus::parseOpenFileArguments(argc, argv);
	
	// If no file to open is passed in command line, create a new default main window. It's shown automatically by CACanorus::addMainWin().
	if (!CACanorus::mainWinCount()) {
		CAMainWin *mainWin = new CAMainWin();
		CACanorus::addMainWin(mainWin);
		mainWin->newDocument();
	}
	
	// Show settings dialog, if needed (eg. MIDI setup when running Canorus for the first time)
	if ( showSettingsPage != CASettingsDialog::UndefinedSettings )
		CASettingsDialog( showSettingsPage, CACanorus::mainWinAt(0) );
	
	QObject::connect( &mainApp, SIGNAL(lastWindowClosed()), CACanorus::autoRecovery(), SLOT(cleanupRecovery()) );  
	return mainApp.exec();
}
