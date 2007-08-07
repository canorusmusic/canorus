/*! 
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "core/settings.h"
#include "core/canorus.h"
#include "interface/rtmididevice.h"

// Define default settings
const bool CASettings::DEFAULT_FINALE_LYRICS_BEHAVIOUR = false;
const QDir CASettings::DEFAULT_DOCUMENTS_DIRECTORY = QDir::home();
const CAFileFormats::CAFileFormatType CASettings::DEFAULT_SAVE_FORMAT = CAFileFormats::CanorusML;
const int CASettings::DEFAULT_AUTO_RECOVERY_INTERVAL = 1;
const QColor CASettings::DEFAULT_BACKGROUND_COLOR = QColor(255, 255, 240);
const QColor CASettings::DEFAULT_FOREGROUND_COLOR = Qt::black;
const QColor CASettings::DEFAULT_SELECTION_COLOR = Qt::red;
const QColor CASettings::DEFAULT_SELECTION_AREA_COLOR = QColor( 255, 0, 80, 70 );
const QColor CASettings::DEFAULT_SELECTED_CONTEXT_COLOR = Qt::blue;
const QColor CASettings::DEFAULT_HIDDEN_ELEMENTS_COLOR = Qt::green;
const QColor CASettings::DEFAULT_DISABLED_ELEMENTS_COLOR = Qt::gray;
const int CASettings::DEFAULT_MIDI_IN_PORT = -1;
const int CASettings::DEFAULT_MIDI_OUT_PORT = -1;

CASettings::CASettings(const QString & fileName, Format format, QObject * parent)
 : QSettings(fileName, format, parent) {
}

CASettings::~CASettings() {
	writeSettings();
}

/*!
	Writes the stored settings to a config file.
*/
void CASettings::writeSettings() {
	setValue( "editor/finalelyricsbehaviour", finaleLyricsBehaviour() );
	setValue( "files/documentsdirectory", documentsDirectory().absolutePath() );
	setValue( "files/defaultsaveformat", defaultSaveFormat() );
	setValue( "files/autorecoveryinterval", autoRecoveryInterval() );
	setValue( "appearance/backgroundcolor", backgroundColor() );
	setValue( "appearance/foregroundcolor", foregroundColor() );
	setValue( "appearance/selectioncolor", selectionColor() );
	setValue( "appearance/selectionareacolor", selectionAreaColor() );
	setValue( "appearance/selectedcontextcolor", selectedContextColor() );
	setValue( "appearance/hiddenelementscolor",hiddenElementsColor() );
	setValue( "appearance/disabledelementscolor", disabledElementsColor() );
	setValue( "rtmidi/midioutport", midiOutPort() );
	setValue( "rtmidi/midiinport", midiInPort() );
	sync();	
}

/*!
	Opens the settings stored in a config file and sets the local values.
	Returns Undefined, if everything went fine or appropriate settings page, if a file or value wasn't set
	and settings window should be shown (eg. setup the MIDI devices the first time).
*/
CASettingsDialog::CASettingsPage CASettings::readSettings() {
	CASettingsDialog::CASettingsPage settingsPage = CASettingsDialog::UndefinedSettings;
	
	// Editor settings
	if ( contains("editor/finalelyricsbehaviour") )
		setFinaleLyricsBehaviour( value("editor/finalelyricsbehaviour").toBool() );
	else
		setFinaleLyricsBehaviour( DEFAULT_FINALE_LYRICS_BEHAVIOUR );
	
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
	
	// Appearance settings
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
	
	// Playback settings
	if ( contains("rtmidi/midiinport") &&
	     value("rtmidi/midiinport").toInt() < CACanorus::midiDevice()->getInputPorts().count()
	   ) {
		setMidiInPort( value("rtmidi/midiinport").toInt() );	
	} else {
		setMidiInPort( DEFAULT_MIDI_IN_PORT );
		settingsPage = CASettingsDialog::PlaybackSettings;
	}
	
	if ( contains("rtmidi/midioutport") &&
	     value("rtmidi/midioutport").toInt() < CACanorus::midiDevice()->getOutputPorts().count()
	   ) {
		setMidiOutPort( value("rtmidi/midioutport").toInt() );	
	} else {
		setMidiOutPort( DEFAULT_MIDI_OUT_PORT );
		settingsPage = CASettingsDialog::PlaybackSettings;
	}
	
	return settingsPage;
}
