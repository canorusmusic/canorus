/** @file ui/midisetupdialog.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "ui/midisetupdialog.h"

CAMidiSetupDialog::CAMidiSetupDialog(QWidget *parent, QMap<int, QString> inPorts, QMap<int, QString> outPorts, int *in, int *out)
	: QDialog(parent)
{
	_inPorts = inPorts;
	_outPorts = outPorts;
	_in = in;
	_out = out;
	
	_dialog.setupUi(this);
	_dialog.midi_in_list->addItem(tr("None"));
	for (int i=0; i<inPorts.values().size(); i++) {
		_dialog.midi_in_list->addItem(inPorts.values().at(i));
		if (*_in==inPorts.keys().at(i))
			_dialog.midi_in_list->setCurrentItem(_dialog.midi_in_list->item(i+1));	//select the previous device
	}
	if (*_in==-1)
		_dialog.midi_in_list->setCurrentItem(_dialog.midi_in_list->item(0));		//select the previous device
	
	_dialog.midi_out_list->addItem(tr("None"));
	for (int i=0; i<outPorts.values().size(); i++) {
		_dialog.midi_out_list->addItem(outPorts.values().at(i));
		if (*_out==outPorts.keys().at(i))
			_dialog.midi_out_list->setCurrentItem(_dialog.midi_out_list->item(i+1));	//select the previous device
	}
	if (*_out==-1)
		_dialog.midi_out_list->setCurrentItem(_dialog.midi_out_list->item(0));			//select the previous device
	
	exec();
}

CAMidiSetupDialog::~CAMidiSetupDialog() {
}

void CAMidiSetupDialog::accept() {
	if (_dialog.midi_in_list->currentIndex().row()==0)
		*_in = -1;
	else
		*_in = _inPorts.keys().at(_dialog.midi_in_list->currentIndex().row()-1);
	
	if (_dialog.midi_out_list->currentIndex().row()==0)
		*_out = -1;
	else
		*_out = _outPorts.keys().at(_dialog.midi_out_list->currentIndex().row()-1);
	
	hide();
}

void CAMidiSetupDialog::reject() {
	hide();
}
