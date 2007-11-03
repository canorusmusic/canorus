/*! 
	Copyright (c) 2006-2007, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QApplication>
#include <QSplashScreen>
#include <QTranslator>
#include <QFont>
#include <QLocale>

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
	
	QPixmap splashPixmap( 400, 300 );
	if ( CACanorus::locateResource("images/splash.png").size() )
		splashPixmap = QPixmap( CACanorus::locateResource("images/splash.png")[0] );
	else if ( CACanorus::locateResource("ui/images/splash.png").size() )
		splashPixmap = QPixmap( CACanorus::locateResource("ui/images/splash.png")[0] );
	
	QSplashScreen splash( splashPixmap );
	QFont font("Century Schoolbook L");
	font.setPixelSize(17);
	splash.setFont(font);
	mainApp.processEvents();
	
	// Set main application properties
	CACanorus::initMain();
	
	// Parse switch and settings command line arguments
	CACanorus::parseSettingsArguments(argc, argv);
	splash.show();
	
	// Load system translation if found
	QList<QString> translationLocations =
		CACanorus::locateResource(QString("lang/") + QLocale::system().name() + ".qm"); // load language_COUNTRY.qm
	if (!translationLocations.size())
		translationLocations = CACanorus::locateResource(QString("lang/") + QLocale::system().name().left(2) + ".qm"); // if not found, load language.qm only
	
	QTranslator translator;
	if (translationLocations.size()) {
		translator.load(translationLocations[0]);
		mainApp.installTranslator(&translator);
	}
	
	if(QLocale::system().language() == QLocale::Hebrew) // \todo add Arabic, etc.
		mainApp.setLayoutDirection(Qt::RightToLeft);
	
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
	
	// Initialize undo/redo stacks
	splash.showMessage( QObject::tr("Initializing Undo/Redo framework", "splashScreen"), Qt::AlignBottom|Qt::AlignLeft, Qt::white );
	mainApp.processEvents();
	CACanorus::initUndo();
	
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
	
	QObject::connect( &mainApp, SIGNAL(lastWindowClosed()), CACanorus::autoRecovery(), SLOT(cleanupRecovery()) );
	
	return mainApp.exec();
}
