/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "core/settings.h"
#ifndef SWIGCPP
#include "canorus.h"
#endif
#include "interface/rtmididevice.h"

// Define default settings
const bool CASettings::DEFAULT_FINALE_LYRICS_BEHAVIOUR = false;
const bool CASettings::DEFAULT_SHADOW_NOTES_IN_OTHER_STAFFS = false;
const bool CASettings::DEFAULT_PLAY_INSERTED_NOTES = true;
const bool CASettings::DEFAULT_AUTO_BAR = true;
const bool CASettings::DEFAULT_USE_NOTE_CHECKER = true;

const QDir CASettings::DEFAULT_DOCUMENTS_DIRECTORY = QDir::home();
const QDir CASettings::DEFAULT_SHORTCUTS_DIRECTORY = QDir( QDir::homePath() + "/.config/Canorus" );
const CAFileFormats::CAFileFormatType CASettings::DEFAULT_SAVE_FORMAT = CAFileFormats::Can;
const int CASettings::DEFAULT_AUTO_RECOVERY_INTERVAL = 1;
const int CASettings::DEFAULT_MAX_RECENT_DOCUMENTS = 15;

#ifndef SWIGCPP
const bool   CASettings::DEFAULT_LOCK_SCROLL_PLAYBACK = true; // scroll while playing
const bool   CASettings::DEFAULT_ANIMATED_SCROLL = true;
const bool   CASettings::DEFAULT_ANTIALIASING = true;
const bool   CASettings::DEFAULT_SHOW_RULER = true;
const QColor CASettings::DEFAULT_BACKGROUND_COLOR = QColor(255, 255, 240);
const QColor CASettings::DEFAULT_FOREGROUND_COLOR = Qt::black;
const QColor CASettings::DEFAULT_SELECTION_COLOR = Qt::red;
const QColor CASettings::DEFAULT_SELECTION_AREA_COLOR = QColor( 255, 0, 80, 70 );
const QColor CASettings::DEFAULT_SELECTED_CONTEXT_COLOR = Qt::blue;
const QColor CASettings::DEFAULT_HIDDEN_ELEMENTS_COLOR = Qt::green;
const QColor CASettings::DEFAULT_DISABLED_ELEMENTS_COLOR = Qt::gray;
#endif

const int CASettings::DEFAULT_MIDI_IN_PORT = -1;
const int CASettings::DEFAULT_MIDI_OUT_PORT = -1;

const CATypesetter::CATypesetterType CASettings::DEFAULT_TYPESETTER = CATypesetter::LilyPond;
#ifdef Q_OS_WIN
const QString                        CASettings::DEFAULT_TYPESETTER_LOCATION = "LilyPond/usr/bin/lilypond.exe";
#elif Q_OS_MAC
const QString                        CASettings::DEFAULT_TYPESETTER_LOCATION = "/Applications/LilyPond.app/Contents/Resources/bin/lilypond";
#else
const QString                        CASettings::DEFAULT_TYPESETTER_LOCATION = "lilypond";
#endif
const bool                           CASettings::DEFAULT_USE_SYSTEM_TYPESETTER = true;
const QString                        CASettings::DEFAULT_PDF_VIEWER_LOCATION = "";
const bool                           CASettings::DEFAULT_USE_SYSTEM_PDF_VIEWER = true;

/*!
	\class CASettings
	\brief Class for storing the Canorus settings

	This class is a model class used for reading and writing the settings in/out
	of the config file.

	The default location of config files are usually $HOME$/.config/Canorus for
	POSIX systems and %HOME%\Application data\Canorus on M$ systems. See
	\fn defaultSettingsPath().

	To use this class simply create it and (optionally) pass a config file name.
	Use readSettings() to read the actual values then. Use getter/setter methods
	for getting the properties and call writeSettings() to store new settings.

	To add a new property:
	1) add a private property (eg. _textColor)
	2) add public getter, setter methods and initial value (eg. getTextColor(), setTextColor(), DEFAULT_TEXT_COLOR)
	3) define the initial value itself in .cpp file
	4) add setValue() sentence in writeSettings() method and setTextColor() in readSettings()
	5) (optionally) create GUI in CASettingsDialog or CAMainWin menu for your new property

	\sa CASettingsDialog
 */

/*!
	Create a new settings instance using the config file \a fileName.
 */
CASettings::CASettings(const QString & fileName, QObject * parent)
 : QSettings(fileName, QSettings::IniFormat, parent) {
	 initSettings();
}

/*!
	Create a new settings instance using the default config file for a local user.
 */
CASettings::CASettings( QObject * parent )
 : QSettings( defaultSettingsPath()+"/canorus.ini", QSettings::IniFormat, parent) {
	 initSettings();
}

void CASettings::initSettings()
{
#ifndef SWIGCPP
    _poEmptyEntry = new CASingleAction( this );
#endif
}

CASettings::~CASettings() {
	writeSettings();
	if( _poEmptyEntry )
	  delete _poEmptyEntry;
	_poEmptyEntry = 0;

#ifndef SWIGCPP
    if( false == _oActionList.isEmpty() )
    {
        CASingleAction *poActionEntry;
        foreach(poActionEntry, _oActionList)
        {
            delete poActionEntry;
            poActionEntry = 0;
        }
        _oActionList.clear();
    }
#endif
}

/*!
	Writes the stored settings to a config file.
*/
void CASettings::writeSettings() {
	setValue( "editor/finalelyricsbehaviour", finaleLyricsBehaviour() );
	setValue( "editor/shadownotesinotherstaffs", shadowNotesInOtherStaffs() );
	setValue( "editor/playinsertednotes", playInsertedNotes() );
	setValue( "editor/autobar", autoBar() );
	setValue( "editor/usenotechecker", useNoteChecker() );
	setValue( "appearance/showruler", showRuler() );

	setValue( "files/documentsdirectory", documentsDirectory().absolutePath() );
	setValue( "files/defaultsaveformat", defaultSaveFormat() );
	setValue( "files/autorecoveryinterval", autoRecoveryInterval() );
	setValue( "files/maxrecentdocuments", maxRecentDocuments() );
#ifndef SWIGCPP
	writeRecentDocuments();

	setValue( "appearance/lockscrollplayback", lockScrollPlayback() );
	setValue( "appearance/animatedscroll", animatedScroll() );
	setValue( "appearance/antialiasing", antiAliasing() );
	setValue( "appearance/backgroundcolor", backgroundColor() );
	setValue( "appearance/foregroundcolor", foregroundColor() );
	setValue( "appearance/selectioncolor", selectionColor() );
	setValue( "appearance/selectionareacolor", selectionAreaColor() );
	setValue( "appearance/selectedcontextcolor", selectedContextColor() );
	setValue( "appearance/hiddenelementscolor",hiddenElementsColor() );
	setValue( "appearance/disabledelementscolor", disabledElementsColor() );
#endif
	setValue( "rtmidi/midioutport", midiOutPort() );
	setValue( "rtmidi/midiinport", midiInPort() );

	setValue( "printing/typesetter", typesetter() );
	setValue( "printing/typesetterlocation", typesetterLocation() );
	setValue( "printing/usesystemdefaulttypesetter", useSystemDefaultTypesetter() );
	setValue( "printing/pdfviewerlocation", pdfViewerLocation() );
	setValue( "printing/usesystemdefaultpdfviewer", useSystemDefaultPdfViewer() );

	sync();
}

/*!
	Opens the settings stored in a config file and sets the local values.
	This function is usually called on application startup to read and validate
	various settings and show the settings window, if needed (eg. setup the MIDI
	devices the first time).
	
	\return 0, if settings validated fine; -1 for playback settings errors.
*/
int CASettings::readSettings() {
	int settingsPage = 0;

	// Editor settings
	if ( contains("editor/finalelyricsbehaviour") )
		setFinaleLyricsBehaviour( value("editor/finalelyricsbehaviour").toBool() );
	else
		setFinaleLyricsBehaviour( DEFAULT_FINALE_LYRICS_BEHAVIOUR );

	if ( contains("editor/shadownotesinotherstaffs") )
		setShadowNotesInOtherStaffs( value("editor/shadownotesinotherstaffs").toBool() );
	else
		setShadowNotesInOtherStaffs( DEFAULT_SHADOW_NOTES_IN_OTHER_STAFFS );

	if ( contains("editor/playinsertednotes") )
		setPlayInsertedNotes( value("editor/playinsertednotes").toBool() );
	else
		setPlayInsertedNotes( DEFAULT_PLAY_INSERTED_NOTES );

	if ( contains("editor/autobar") )
		setAutoBar( value("editor/autobar").toBool() );
	else
		setAutoBar( DEFAULT_AUTO_BAR );

	if ( contains("editor/usenotechecker") )
		setUseNoteChecker( value("editor/usenotechecker").toBool() );
	else
		setUseNoteChecker( DEFAULT_USE_NOTE_CHECKER );

	// Saving/Loading settings
	if ( contains("files/documentsdirectory") )
		setDocumentsDirectory( value("files/documentsdirectory").toString() );
	else
		setDocumentsDirectory( DEFAULT_DOCUMENTS_DIRECTORY );

	if ( contains("files/defaultsaveformat") )
		setDefaultSaveFormat( static_cast<CAFileFormats::CAFileFormatType>(value("files/defaultsaveformat").toInt()) );
	else
		setDefaultSaveFormat( DEFAULT_SAVE_FORMAT );

	if ( contains("files/autorecoveryinterval") )
		setAutoRecoveryInterval( value("files/autorecoveryinterval").toInt() );
	else
		setAutoRecoveryInterval( DEFAULT_AUTO_RECOVERY_INTERVAL );

	// Recently opened files
	if ( contains("files/maxrecentdocuments") )
		setMaxRecentDocuments( value("files/maxrecentdocuments").toInt() );
	else
		setMaxRecentDocuments( DEFAULT_MAX_RECENT_DOCUMENTS );

#ifndef SWIGCPP
	readRecentDocuments();

	// Appearance settings
	if ( contains("appearance/lockscrollplayback") )
		setLockScrollPlayback( value("appearance/lockscrollplayback").toBool() );
	else
		setLockScrollPlayback( DEFAULT_LOCK_SCROLL_PLAYBACK );

	if ( contains("appearance/animatedscroll") )
		setAnimatedScroll( value("appearance/animatedscroll").toBool() );
	else
		setAnimatedScroll( DEFAULT_ANIMATED_SCROLL );

	if ( contains("appearance/antialiasing") )
		setAntiAliasing( value("appearance/antialiasing").toBool() );
	else
		setAntiAliasing( DEFAULT_ANTIALIASING );

	if ( contains("appearance/backgroundcolor") )
		setBackgroundColor( value("appearance/backgroundcolor").value<QColor>() );
	else
		setBackgroundColor( DEFAULT_BACKGROUND_COLOR );

	if ( contains("appearance/foregroundcolor") )
		setForegroundColor( value("appearance/foregroundcolor").value<QColor>() );
	else
		setForegroundColor( DEFAULT_FOREGROUND_COLOR );

	if ( contains("appearance/selectioncolor") )
		setSelectionColor( value("appearance/selectioncolor").value<QColor>() );
	else
		setSelectionColor( DEFAULT_SELECTION_COLOR );

	if ( contains("appearance/selectionareacolor") )
		setSelectionAreaColor( value("appearance/selectionareacolor").value<QColor>() );
	else
		setSelectionAreaColor( DEFAULT_SELECTION_AREA_COLOR );

	if ( contains("appearance/selectedcontextcolor") )
		setSelectedContextColor( value("appearance/selectedcontextcolor").value<QColor>() );
	else
		setSelectedContextColor( DEFAULT_SELECTED_CONTEXT_COLOR );

	if ( contains("appearance/hiddenelementscolor") )
		setHiddenElementsColor( value("appearance/hiddenelementscolor").value<QColor>() );
	else
		setHiddenElementsColor( DEFAULT_HIDDEN_ELEMENTS_COLOR );

	if ( contains("appearance/disabledelementscolor") )
		setDisabledElementsColor( value("appearance/disabledelementscolor").value<QColor>() );
	else
		setDisabledElementsColor( DEFAULT_DISABLED_ELEMENTS_COLOR );
	
	if ( contains("appearance/showruler") )
		setShowRuler( value("appearance/showruler").toBool() );
	else
		setShowRuler( DEFAULT_SHOW_RULER );

#endif
	// Playback settings
	if ( contains("rtmidi/midiinport")
#ifndef SWIGCPP
		&& value("rtmidi/midiinport").toInt() < CACanorus::midiDevice()->getInputPorts().count()
#endif
	   ) {
		setMidiInPort( value("rtmidi/midiinport").toInt() );
	} else {
		setMidiInPort( DEFAULT_MIDI_IN_PORT );
		settingsPage = -1;
	}

	if ( contains("rtmidi/midioutport")
#ifndef SWIGCPP
		 && value("rtmidi/midioutport").toInt() < CACanorus::midiDevice()->getOutputPorts().count()
#endif
	   ) {
		setMidiOutPort( value("rtmidi/midioutport").toInt() );
	} else {
		setMidiOutPort( DEFAULT_MIDI_OUT_PORT );
		settingsPage = -1;
	}

	// Printing settings
	if ( contains("printing/typesetter") )
		setTypesetter( static_cast<CATypesetter::CATypesetterType>(value("printing/typesetter").toInt()) );
	else
		setTypesetter( DEFAULT_TYPESETTER );

	if ( contains("printing/typesetterlocation") )
		setTypesetterLocation( value("printing/typesetterlocation").toString() );
	else
		setTypesetterLocation( DEFAULT_TYPESETTER_LOCATION );

	if ( contains("printing/usesystemdefaulttypesetter") )
		setUseSystemDefaultTypesetter( value("printing/usesystemdefaulttypesetter").toBool() );
	else
		setUseSystemDefaultTypesetter( DEFAULT_USE_SYSTEM_TYPESETTER );

	if ( contains("printing/pdfviewerlocation") )
		setPdfViewerLocation( value("printing/pdfviewerlocation").toString() );
	else
		setPdfViewerLocation( DEFAULT_PDF_VIEWER_LOCATION );

	if ( contains("printing/usesystemdefaultpdfviewer") )
		setUseSystemDefaultPdfViewer( value("printing/usesystemdefaultpdfviewer").toBool() );
	else
		setUseSystemDefaultPdfViewer( DEFAULT_USE_SYSTEM_PDF_VIEWER );

	return settingsPage;

	// Action / Command settings
	if ( contains("action/shortcutsdirectory") )
		setLatestShortcutsDirectory( value("action/shortcutsdirectory").toString() );
	else
		setLatestShortcutsDirectory( DEFAULT_SHORTCUTS_DIRECTORY );
}

void CASettings::setMidiInPort(int in) {
	_midiInPort = in;
#ifndef SWIGCPP
	if (CACanorus::midiDevice()) {
		CACanorus::midiDevice()->closeInputPort();
		CACanorus::midiDevice()->openInputPort( midiInPort() );
	}
#endif
}

#ifndef SWIGCPP

/*!
  Search one single action in the list of actions (-1: entry not found)
  Returns an empty action element when the command was not found
*/
int CASettings::getSingleAction(QString oCommandName, QAction *&poResAction)
{
    CASingleAction *poEntryAction;
	for (int i=0; i < _oActionList.count(); i++) {
        poEntryAction = _oActionList[i];
        if( poEntryAction->getCommandName() == oCommandName )
        {
            poResAction = poEntryAction;
			return i;
        }
	}
	poResAction = _poEmptyEntry;
    return -1;
}

int CASettings::getSingleAction(QString oCommandName, CASingleAction *&poResAction)
{
    CASingleAction *poEntryAction;
    for (int i=0; i < _oActionList.count(); i++) {
        poEntryAction = _oActionList[i];
        if( poEntryAction->getCommandName() == oCommandName )
        {
            poResAction = poEntryAction;
            return i;
        }
    }
    poResAction = _poEmptyEntry;
    return -1;
}

/*!
 Updates an action in the action list
 Return 'true' if the update was successfull
 Warning: 1) The action will not be copied
          2) Only shortcut and no midi information is updated
          3) Description cannot be updated
*/
bool CASettings::setSingleAction(QAction oSingleAction, int iPos)
{
	bool bRet = false;
	if( iPos >= 0 && iPos < _oActionList.count() ) {
        _oActionList[iPos]->setCommandName( oSingleAction.objectName() );
        _oActionList[iPos]->setShortCutAsString( oSingleAction.shortcut().toString() );
		bRet = true;
	}
	return bRet;
}

/*!
 Takes a complete action list as it's own
 Manually: Removes all elements and copies every single in the own list
 Else: According to Qt doc "assigns the other list to this list"
 Warning: The actions themselves cannot be copied!
*/
void CASettings::setActionList(QList<CASingleAction *> &oActionList)
{
#ifdef COPY_ACTIONLIST_ELEMS_MANUALLY
	_oActionList.clear();
    CASingleAction *pActionEntry;
	for (int i=0; i < oActionList.count(); i++) {
        pActionEntry = &getSingleAction(i, oActionList);
        addSingleAction(*pActionEntry);
	}
#else
	_oActionList = oActionList;
#endif
}

/*!
 Adds a single action to the action list
 Warning: The action will be referenced!
*/
void CASettings::addSingleAction(CASingleAction &oSingleAction)
{
    CASingleAction *pActionEntry = new CASingleAction(0); // parent ?
    pActionEntry->setCommandName( oSingleAction.getCommandName() );
    pActionEntry->setDescription( oSingleAction.getDescription() );
    pActionEntry->setShortCutAsString( oSingleAction.getShortCutAsString() );
    pActionEntry->setMidiKeySequence( oSingleAction.getMidiKeySequence() );
    pActionEntry->setActionGroup( oSingleAction.actionGroup() );
    pActionEntry->setAutoRepeat( oSingleAction.autoRepeat() );
    pActionEntry->setCheckable( oSingleAction.isCheckable() );
    pActionEntry->setChecked( oSingleAction.isChecked() );
    pActionEntry->setData( oSingleAction.data() );
#ifdef COPY_ACTIONLIST_ELEMS_MANUALLY
    _oActionList.append( pActionEntry );
#else
    _oActionList.append( &oSingleAction );
#endif
}

/*!
 Removes a single action from the action list
 Return 'true' when succesfull
 Warning: The action itself is not deleted!
*/
bool CASettings::deleteSingleAction(QString oCommand)
{
    CASingleAction *poResAction;
	bool bRet = false;
	int iPos = getSingleAction(oCommand, poResAction);
	if( iPos >= 0 ) // Double entries should not be in the list
	{
		_oActionList.removeOne( poResAction );
		bRet = true;
	}
#ifdef COPY_ACTIONLIST_ELEMS_MANUALLY
    delete poResAction;
#endif
	return bRet;
}

void CASettings::readRecentDocuments() {
	for ( int i=0; contains( QString("files/recentdocument") + QString::number(i) ); i++ )
		CACanorus::addRecentDocument( value(QString("files/recentdocument") + QString::number(i)).toString() );
}

void CASettings::writeRecentDocuments() {
	for ( int i=0; contains( QString("files/recentdocument") + QString::number(i) ); i++ )
		remove( QString("files/recentdocument") + QString::number(i) );

	for ( int i=0; i<CACanorus::recentDocumentList().size(); i++ )
		setValue( QString("files/recentdocument") + QString::number(i), CACanorus::recentDocumentList()[i] );
}
#endif // SWIGCPP

/*!
	Returns the default settings path. This function is static and is used when no config
	filename is specified or when a plugin wants a settings directory to store its own settings in.
 */
const QString CASettings::defaultSettingsPath() {
#ifdef Q_WS_WIN	// M$ is of course an exception
	return QDir::homePath()+"/Application Data/Canorus";
#else	// POSIX systems use the same config file path
	return QDir::homePath()+"/.config/Canorus";
#endif
}
