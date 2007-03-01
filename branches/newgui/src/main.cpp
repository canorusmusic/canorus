/** \file main.cpp
 * 
 * Copyright (c) 2006-2007, Reinhard Katzmann, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "ui/mainwin.h"
#include "core/canorus.h"
#include <QApplication>

/*!
	Main function. This is the first function called when Canorus is run.
	It initializes CACanorus class and creates the main window.
*/
int main(int argc, char *argv[]) {
	QApplication mainApp(argc, argv);
	
	// Set main application properties
	CACanorus::initMain();
	
	// Load config file
	CACanorus::initSettings();
	
	// Parse command line arguments - creates a main window of a document to open if passed in command line
	CACanorus::parseAppArguments(argc, argv);
	
	// If no file to open is passed in command line, create a new default main window. It's shown automatically by CACanorus::addMainWin().
	if (!CACanorus::mainWinCount())
		CACanorus::addMainWin(new CAMainWin());
	
	return mainApp.exec();
}
