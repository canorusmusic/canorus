/*! 
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

// Python.h needs to be loaded first!
#include "scripting/swigpython.h"
#include "core/canorus.h"

#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>

#include "interface/rtmididevice.h"
#include "ui/settingsdialog.h"
#include "scripting/swigruby.h"
#include "core/sheet.h"
#include "core/settings.h"
#include "core/undo.h"

// define private static members
QList<CAMainWin*> CACanorus::_mainWinList;
CASettings *CACanorus::_settings;
QString CACanorus::_settingsPath;
CAAutoRecovery *CACanorus::_autoRecovery;
CAMidiDevice *CACanorus::_midiDevice;
CAUndo *CACanorus::_undo;

/*!
	Locates a resource named fileName (relative path) and returns its absolute path of the file
	if found in the following order:
		- passed path as an argument to exe
		- path in user's config file
		- current dir
		- exe dir
		- DEFAULT_DATA_DIR set by compiler
	
	\sa locateResourceDir()
*/
QList<QString> CACanorus::locateResource(const QString fileName) {
	QList<QString> paths;
	//! \todo Config file implementation
	//! \todo Application path argument
	QString curPath;
	
	// Try absolute path
	curPath = fileName;
	if (QFileInfo(curPath).exists())
		paths << QFileInfo(curPath).absoluteFilePath();
	
	// Try current working directory
	curPath = QDir::currentPath() + "/" + fileName;
	if (QFileInfo(curPath).exists())
		paths << QFileInfo(curPath).absoluteFilePath();
	
	// Try application exe directory
	curPath = QCoreApplication::applicationDirPath() + "/" + fileName;
	if (QFileInfo(curPath).exists())
		paths << QFileInfo(curPath).absoluteFilePath();
	
#ifdef DEFAULT_DATA_DIR
	// Try compiler defined DEFAULT_DATA_DIR constant (useful on Unix OSes)
	curPath = QString(DEFAULT_DATA_DIR) + "/" + fileName;
	if (QFileInfo(curPath).exists())
		paths << QFileInfo(curPath).absoluteFilePath();
	
#endif
	
	// Remove duplicates. Is there a faster way to do this?
	return paths.toSet().toList();
}

/*!
	Finds the resource named fileName and returns its absolute directory.
	
	\sa locateResource()
*/
QList<QString> CACanorus::locateResourceDir(const QString fileName) {
	QList<QString> paths = CACanorus::locateResource(fileName);
	for (int i=0; i<paths.size(); i++)
		paths[i] = paths[i].left(paths[i].lastIndexOf("/"));
	
	// Remove duplicates. Is there a faster way to do this?
	return paths.toSet().toList();
}

/*!
	Initializes application properties like application name, home page etc.
*/
void CACanorus::initMain() {
	_autoRecovery = 0;
	
	// Init main application properties
	QCoreApplication::setOrganizationName("Canorus");
	QCoreApplication::setOrganizationDomain("canorus.org");
	QCoreApplication::setApplicationName("Canorus");
	setMidiDevice( new CARtMidiDevice() );	
}

void CACanorus::initCommonGUI() {
	// Initialize main window's load/save/import/export dialogs
	CAMainWin::uiSaveDialog = new QFileDialog(0, QObject::tr("Choose a file to save"), settings()->documentsDirectory().absolutePath());
	CAMainWin::uiSaveDialog->setFileMode(QFileDialog::AnyFile);
	CAMainWin::uiSaveDialog->setAcceptMode( QFileDialog::AcceptSave );
	CAMainWin::uiSaveDialog->setFilter( CAFileFormats::CANORUSML_FILTER );
	CAMainWin::uiSaveDialog->selectFilter( CAFileFormats::getFilter( settings()->defaultSaveFormat() ) );
	
	CAMainWin::uiOpenDialog = new QFileDialog(0, QObject::tr("Choose a file to open"), settings()->documentsDirectory().absolutePath());
	CAMainWin::uiOpenDialog->setFileMode( QFileDialog::ExistingFile );
	CAMainWin::uiOpenDialog->setAcceptMode( QFileDialog::AcceptOpen );
	CAMainWin::uiOpenDialog->setFilter( CAFileFormats::CANORUSML_FILTER );
	
	CAMainWin::uiExportDialog = new QFileDialog(0, QObject::tr("Choose a file to export"), settings()->documentsDirectory().absolutePath());
	CAMainWin::uiExportDialog->setFileMode(QFileDialog::AnyFile);
	CAMainWin::uiExportDialog->setAcceptMode( QFileDialog::AcceptSave );
	CAMainWin::uiExportDialog->setFilter( CAFileFormats::LILYPOND_FILTER );
  	
	CAMainWin::uiImportDialog = new QFileDialog(0, QObject::tr("Choose a file to import"), settings()->documentsDirectory().absolutePath());
	CAMainWin::uiImportDialog->setFileMode( QFileDialog::ExistingFile );
	CAMainWin::uiImportDialog->setAcceptMode( QFileDialog::AcceptOpen );
}


/*!
	Opens Canorus config file and loads the settings.
	Config file is always INI file in user's home directory.
	No native formats are used (Windows registry etc.) - this is provided for easier transition of settings between the platforms.
	
	\sa settings() 
*/
CASettingsDialog::CASettingsPage CACanorus::initSettings() {
#ifdef Q_WS_WIN	// M$ is of course an exception
	_settingsPath = QDir::homePath()+"/Application Data/Canorus";
#else	// POSIX systems use the same config file path
	_settingsPath = QDir::homePath()+"/.config/Canorus";
#endif
	
	_settings = new CASettings( settingsPath()+"/canorus.ini", QSettings::IniFormat );
	
	return _settings->readSettings();
}

/*!
	Initializes scripting and plugins subsystem.
*/
void CACanorus::initScripting() {
#ifdef USE_RUBY	
	CASwigRuby::init();
#endif
#ifdef USE_PYTHON
	CASwigPython::init();
#endif
}

/*!
	Initializes recovery saving.
*/
void CACanorus::initAutoRecovery() {
	_autoRecovery = new CAAutoRecovery();
}

void CACanorus::initUndo() {
	_undo = new CAUndo();
}

/*!
	Adds and initializes already created main window to main window list and shows it (default) if \a show is set.
	
	\sa removeMainWin(), mainWinAt()
*/
void CACanorus::addMainWin( CAMainWin *w, bool show ) {
	_mainWinList << w;
	
	if (show)
		w->show();
}

/*!
	Parses the switches and settings command line arguments to application.
	This function sets any settings passed in command line.
	
	\sa parseOpenFileArguments()
*/
void CACanorus::parseSettingsArguments(int argc, char *argv[]) {

}

/*!
	This function parses any arguments which doesn't look like switch or a setting.
	It creates a new main window and opens a file if a file is passed in the command line.
*/
void CACanorus::parseOpenFileArguments(int argc, char *argv[]) {
	for (int i=1; i<argc; i++) {
		if (argv[i][0]!='-') { /// automatically treat any argument which doesn't start with '-' to be a file name - \todo
			// passed is not the switch but a file name
			if (!CACanorus::locateResource(argv[i]).size())
				continue;
			QString fileName = CACanorus::locateResource(argv[i]).at(0);
			
			CAMainWin *mainWin = new CAMainWin();
			CACanorus::addMainWin(mainWin);
			mainWin->openDocument(fileName);
		}
	}
}

/*!
	\fn int CACanorus::mainWinCount()
	
	Returns the number of all main windows.
*/

/*!
	Returns number of main windows which have the given document opened.
*/
int CACanorus::mainWinCount(CADocument *doc) {
	int count=0;
	for (int i=0; i<_mainWinList.size(); i++)
		if (_mainWinList[i]->document()==doc)
			count++;
	
	return count;
}

/*!
	Rebuilds main windows with the given \a document and its viewports showing the given \a sheet.
	Rebuilds all viewports if no sheet is null.
	
	\sa rebuildUI(CADocument*), CAMainWin::rebuildUI()
*/
void CACanorus::rebuildUI( CADocument *document, CASheet *sheet ) {
	for (int i=0; i<mainWinCount(); i++)
		if ( mainWinAt(i)->document()==document )
			mainWinAt(i)->rebuildUI(sheet);
}

/*!
	Rebuilds main windows with the given \a document.
	Rebuilds all main windows, if \a document is not given or null.
	
	\sa rebuildUI(CADocument*, CASheet*), CAMainWin::rebuildUI()
*/
void CACanorus::rebuildUI( CADocument *document ) {
	for (int i=0; i<mainWinCount(); i++) {
		if ( document && mainWinAt(i)->document()==document ) {
			mainWinAt(i)->rebuildUI();
		} else if ( !document )
			mainWinAt(i)->rebuildUI();
	}
}

/*!
	Finds and returns a list of main windows containing the given document.
*/
QList<CAMainWin*> CACanorus::findMainWin(CADocument *document) {
	QList<CAMainWin*> mainWinList;
	for (int i=0; i<mainWinCount(); i++)
		if (mainWinAt(i)->document()==document)
			mainWinList << mainWinAt(i);
	
	return mainWinList;
}
