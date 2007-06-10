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
#include <QSettings>

#include "interface/rtmididevice.h"
#include "ui/midisetupdialog.h"
#include "scripting/swigruby.h"
#include "core/sheet.h"

// define private static members
QList<CAMainWin*> CACanorus::_mainWinList;
QSettings *CACanorus::_settings;
CAMidiDevice *CACanorus::_midiDevice;
int CACanorus::_midiOutPort;
int CACanorus::_midiInPort;
QHash< CADocument*, QUndoStack* > CACanorus::_undoStack;
CAUndoCommand *CACanorus::_undoCommand;
QHash< QUndoStack*, CAUndoCommand* > CACanorus::_lastUndoCommand;
QHash< CAUndoCommand*, CAUndoCommand* > CACanorus::_prevUndoCommands;

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
	_undoCommand = 0;
	
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
	Creates MIDI device and loads port numbers. If no port number settings are stored in the config
	file, it brings up the MIDI setup dialog.
*/
void CACanorus::initMidi() {
	setMidiDevice(new CARtMidiDevice());
	
	if (CACanorus::settings()->contains("rtmidi/defaultoutputport") &&
	    CACanorus::settings()->contains("rtmidi/defaultinputport") ) {
		setMidiInPort(CACanorus::settings()->value("rtmidi/defaultinputport").toInt());
		if (midiInPort() >= midiDevice()->getInputPorts().count())
			setMidiInPort(-1);

		setMidiOutPort(CACanorus::settings()->value("rtmidi/defaultoutputport").toInt());
		if (midiOutPort() >= midiDevice()->getOutputPorts().count())
			setMidiOutPort(-1);
			
	} else {
		setMidiInPort(-1);
		setMidiOutPort(-1);
		CAMidiSetupDialog();
	}
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

/*!
	Deletes the undoStack object for the given document.
	This should be called at the end where no main windows are pointing to the given document anymore.
*/
void CACanorus::deleteUndoStack( CADocument *doc ) {
	clearUndoCommand();
	QUndoStack *stack = undoStack(doc);
	_lastUndoCommand.remove(stack);
	delete stack;
	removeUndoStack( doc );
}

/*!
	Call this to add an undo state (created by createUndoCommand()) to the stack.
	
	\warning This function is not thread-safe. createUndoCommand() and pushUndoCommand() should be called from the same thread.
*/
void CACanorus::pushUndoCommand() {
	if (!_undoCommand)
		return;
	
	CADocument *d = (_undoCommand->getRedoDocument()?_undoCommand->getRedoDocument():_undoCommand->getRedoSheet()->document());
	_undoStack[d]->push( _undoCommand ); // push the command on stack and delete commands after it if any (also updated lastUndoCommand hash needed later)
	CAUndoCommand *lastUndoCommand = _lastUndoCommand[_undoStack[d]];
	
	if (lastUndoCommand) {
		if (_undoCommand->getRedoSheet() && lastUndoCommand->getRedoSheet())
			lastUndoCommand->setRedoSheet( _undoCommand->getUndoSheet() );
		else
		if (_undoCommand->getRedoDocument() && lastUndoCommand->getRedoSheet())
			lastUndoCommand->setRedoSheet( _undoCommand->getUndoDocument()->sheetAt(_undoCommand->getRedoDocument()->sheetList().indexOf( lastUndoCommand->getRedoSheet()) ));
		else
		if (_undoCommand->getRedoDocument() && lastUndoCommand->getRedoDocument())
			lastUndoCommand->setRedoDocument( _undoCommand->getUndoDocument() );
		_prevUndoCommands[_undoCommand] = lastUndoCommand;
	}
	
	_lastUndoCommand[ undoStack(d) ] = _undoCommand;
	_undoCommand=0;
}

/*!
	This function is called when Undo commands are deleted and updates the _lastUndoCommand
	which holds the last command after which the undo command will be added by pushUndoCommand().
*/
void CACanorus::updateLastUndoCommand( CAUndoCommand *c ) {
	if (!_prevUndoCommands.contains(c))
		return;
	
	CADocument *doc = (c->getUndoDocument()?c->getUndoDocument():c->getUndoSheet()->document());
	_lastUndoCommand[ undoStack(doc) ] = _prevUndoCommands[c];
	_prevUndoCommands.remove(c);
}

/*!
	Destroys the undo command if decided not to be put on the stack.
	Does nothing if undo command is null.
*/
void CACanorus::clearUndoCommand() {
	if ( _undoCommand ) {
		_undoCommand->setUndoSheet(0); _undoCommand->setRedoSheet(0);
		_undoCommand->setUndoDocument(0); _undoCommand->setRedoDocument(0);
		delete _undoCommand;
		_undoCommand = 0;
	}
}

/*!
	Creates an undo command which is later put on the stack.
	This function is usually called when making changes to the sheet in the score -
	all changes ranging from creation/removal/editing of music elements and contexts to changing sheet properties. 
	
	\warning This function is not thread-safe. createUndoCommand() and pushUndoCommand() should be called from the same thread.
*/
void CACanorus::createUndoCommand( CASheet *s, QString text ) {
	clearUndoCommand();
	_undoCommand = new CAUndoCommand( s, text );
}

/*!
	Creates an undo command which is later put on the stack.
	This function is usually called when making changes to the document in the score -
	all changes ranging from creation/removal of sheets and editing document properties.
	
	\warning This function is not thread-safe. createUndoCommand() and pushUndoCommand() should be called from the same thread.
*/
void CACanorus::createUndoCommand( CADocument *d, QString text ) {
	clearUndoCommand();
	_undoCommand = new CAUndoCommand( d, text );
}

/*!
	\fn void CACanorus::addMainWin(CAMainWin *window, bool show=true)
	
	Adds an already created main window to main window list and shows it (default) if \i show is set.
	
	\sa removeMainWin(), mainWinAt()
*/
