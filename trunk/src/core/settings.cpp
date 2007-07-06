/*! 
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "core/settings.h"
#include "ui/midisetupdialog.h"
#include "core/canorus.h"
#include "interface/rtmididevice.h"

// Define default settings
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
	setValue("rtmidi/defaultoutputport", midiOutPort());
	setValue("rtmidi/defaultinputport", midiInPort());
	sync();	
}

/*!
	Opens the settings stored in a config file and sets the local values.
	Returns False, if everything went fine or True, if a file or value wasn't set and settings window
	should be shown (eg. setup the MIDI devices the first time).
*/
bool CASettings::readSettings() {
	bool showSettingsWin = false;
	
	if ( contains("rtmidi/defaultinputport") &&
	     value("rtmidi/defaultinputport").toInt() < CACanorus::midiDevice()->getInputPorts().count()
	   ) {
		setMidiInPort( value("rtmidi/defaultinputport").toInt() );	
	} else {
		setMidiInPort( DEFAULT_MIDI_IN_PORT );
		showSettingsWin = true;
	}
	
	if ( contains("rtmidi/defaultoutputport") &&
	     value("rtmidi/defaultoutputport").toInt() < CACanorus::midiDevice()->getOutputPorts().count()
	   ) {
		setMidiOutPort( value("rtmidi/defaultoutputport").toInt() );	
	} else {
		setMidiOutPort( DEFAULT_MIDI_OUT_PORT );
		showSettingsWin = true;
	}
	
	return showSettingsWin;
}
