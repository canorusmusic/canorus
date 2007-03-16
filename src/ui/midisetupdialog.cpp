/** @file ui/midisetupdialog.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QSettings>

// Python.h needs to be loaded first!
#include "core/canorus.h"
#include "ui/midisetupdialog.h"
#include "interface/mididevice.h"

CAMidiSetupDialog::CAMidiSetupDialog(QWidget *parent) : QDialog(parent) {
	_inPorts = CACanorus::midiDevice()->getInputPorts();
	_outPorts = CACanorus::midiDevice()->getOutputPorts();
	
	_dialog.setupUi(this);
	_dialog.midi_in_list->addItem(tr("None"));
	for (int i=0; i<_inPorts.values().size(); i++) {
		_dialog.midi_in_list->addItem(_inPorts.values().at(i));
		if (CACanorus::midiInPort()==_inPorts.keys().at(i))
			_dialog.midi_in_list->setCurrentItem(_dialog.midi_in_list->item(i+1));	//select the previous device
	}
	if (CACanorus::midiInPort()==-1)
		_dialog.midi_in_list->setCurrentItem(_dialog.midi_in_list->item(0));		//select the previous device
	
	_dialog.midi_out_list->addItem(tr("None"));
	for (int i=0; i<_outPorts.values().size(); i++) {
		_dialog.midi_out_list->addItem(_outPorts.values().at(i));
		if (CACanorus::midiOutPort()==_outPorts.keys().at(i))
			_dialog.midi_out_list->setCurrentItem(_dialog.midi_out_list->item(i+1));	//select the previous device
	}
	if (CACanorus::midiOutPort()==-1)
		_dialog.midi_out_list->setCurrentItem(_dialog.midi_out_list->item(0));			//select the previous device
	
	exec();
}

CAMidiSetupDialog::~CAMidiSetupDialog() {
}

void CAMidiSetupDialog::accept() {
	if (_dialog.midi_in_list->currentIndex().row()==0)
		CACanorus::setMidiInPort(-1);
	else
		CACanorus::setMidiInPort(_inPorts.keys().at(_dialog.midi_in_list->currentIndex().row()-1));
	
	if (_dialog.midi_out_list->currentIndex().row()==0)
		CACanorus::setMidiOutPort(-1);
	else
		CACanorus::setMidiOutPort(_outPorts.keys().at(_dialog.midi_out_list->currentIndex().row()-1));
	
	CACanorus::settings()->setValue("rtmidi/defaultoutputport", CACanorus::midiOutPort());
	CACanorus::settings()->setValue("rtmidi/defaultinputport", CACanorus::midiInPort());
	
	hide();
}

void CAMidiSetupDialog::reject() {
	hide();
}
