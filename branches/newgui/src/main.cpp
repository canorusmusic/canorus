/** \file main.cpp
 * 
 * Copyright (c) 2006-2007, Reinhard Katzmann, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QApplication>

#include "ui/mainwin.h"
#include "core/canorus.h"
#include "interface/pluginmanager.h"

/*!
	Main function. This is the first function called when Canorus is run.
	It initializes CACanorus class and creates the main window.
*/
int main(int argc, char *argv[]) {
	QApplication mainApp(argc, argv);
	
	// Set main application properties
	CACanorus::initMain();
	
	// Parse switch and settings command line arguments
	CACanorus::parseSettingsArguments(argc, argv);
	
	// Load config file
	CACanorus::initSettings();
	
	// Create MIDI device
	CACanorus::initMidi();
	
	// Enable scripting and plugins subsystem
	CACanorus::initScripting();
	
	// Finds all the plugins
	CAPluginManager::readPlugins();
	
	// Creates a main window of a document to open if passed in command line
	CACanorus::parseOpenFileArguments(argc, argv);
	
	// If no file to open is passed in command line, create a new default main window. It's shown automatically by CACanorus::addMainWin().
	if (!CACanorus::mainWinCount()) {
		CAMainWin *mainWin = new CAMainWin();
		CACanorus::addMainWin(mainWin);
		mainWin->newDocument();
	}
	
	return mainApp.exec();
}
