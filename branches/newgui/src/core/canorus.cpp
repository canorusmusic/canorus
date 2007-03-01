/** \file core/canorus.h
 * 
 * Copyright (c) 2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QSettings>

#include "core/canorus.h"

// define private static members
QList<CAMainWin*> CACanorus::_mainWinList;
QSettings *CACanorus::_settings;

/*!
	Locates a resource named fileName and returns its absolute path.
	
	\sa locateResourceDir()
*/
QString CACanorus::locateResource(const QString fileName) {
	//! \todo Config file implementation
	//! \todo Application path argument
	QString curPath;
	
	// Try current working directory
	curPath = QDir::currentPath() + "/" + fileName;
	if (QFile(curPath).exists())
		return curPath;
	
	// Try application exe directory
	curPath = QCoreApplication::applicationDirPath() + "/" + fileName;
	if (QFile(curPath).exists())
		return curPath;
	
#ifdef DEFAULT_DATA_DIR
	// Try compiler defined DEFAULT_DATA_DIR constant (useful for Linux OSes)
	curPath = QString(DEFAULT_DATA_DIR) + "/" + fileName;
	if (QFile(curPath).exists())
		return curPath;
	
#endif
	// Else, if file not found, return empty string
	return QString("");
}

/*!
	Finds the resource named fileName and returns its absolute directory.
	
	\sa locateResource()
*/
QString CACanorus::locateResourceDir(const QString fileName) {
	QString path = CACanorus::locateResource(fileName);
	if (!path.isEmpty())
		return path.left(path.lastIndexOf("/"));
	else
		return "";
}

/*!
	Initializes application properties like application name, home page etc.
*/
void CACanorus::initMain() {
	// Init main application properties
	QCoreApplication::setOrganizationName("Canorus");
	QCoreApplication::setOrganizationDomain("canorus.org");
	QCoreApplication::setApplicationName("Canorus");
}

/*!
	Opens Canorus config file and loads the settings.
	Config file is always INI file in user's home directory.
	No native formats are used (Windows registry etc.) - this is provided for easier transition of settings between the platforms.
	
	\sa settings() 
*/
void CACanorus::initSettings() {
#ifdef Q_WS_WIN	// M$ is of course an exception
	_settings = new QSettings(QDir::homePath()+"/Application Data/Canorus/canorus.ini", QSettings::IniFormat);
#else	// POSIX systems use the same config file path
	_settings = new QSettings(QDir::homePath()+"/.config/Canorus/canorus.ini", QSettings::IniFormat);
#endif		
}

/*!
	Parses the given command line arguments to application.
	This function sets any settings passed in command line.
	It also creates a new main window and opens a file if a file is passed in the command line.
*/
void CACanorus::parseAppArguments(int argc, char *argv[]) {
	for (int i=1; i<argc; i++) {
		if (argv[i][0]!='-') {
			// passed is not the switch but a file name
			QString fileName = CACanorus::locateResource(argv[i]);
			if (fileName.isEmpty())
				continue;
			
			CAMainWin *mainWin = new CAMainWin();
			CACanorus::addMainWin(mainWin);
			mainWin->openDocument(fileName);
		}
	}
}
