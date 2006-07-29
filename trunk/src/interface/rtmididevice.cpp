/** @file rtmididevice.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "rtmididevice.h"
#include "rtmidi/RtMidi.h"

//my first usable Timidity port is port #2 -Matevz
#define DEFAULT_DEVICE_PORT 1

CARtMidiDevice::CARtMidiDevice() {
	_out = 0;
	_in = 0;
	
	try {
		_out = new RtMidiOut();
	}
	catch (RtError &error) {
		error.printMessage();
	}

	// Check outputs.
	int nPorts = _out->getPortCount();
	if (nPorts > 0) {
		try {
			_out->openPort(DEFAULT_DEVICE_PORT);
		}
		catch (RtError &error) {
			error.printMessage();
		}
	}
}

CARtMidiDevice::~CARtMidiDevice() {
	if (_out) delete _out;
	if (_in) delete _in;
}

void CARtMidiDevice::send(std::vector<unsigned char> *message) {
	_out->sendMessage( message );
}
