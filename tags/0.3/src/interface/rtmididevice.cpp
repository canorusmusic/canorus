/*!
	Copyright (c) 2006, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QVector>

#include "interface/rtmididevice.h"
#include "rtmidi/RtMidi.h"

/*!
	\class CARtMidiDevice
	\brief Canorus wrapper for RtMidi library
	CARtMidiDevice is a Canorus wrapper class for a cross-platform MIDI library
	RtMidi written by Gary P. Scavone (http://www.music.mcgill.ca/~gary/rtmidi/).
	
	Usage:
	1) When created, Input and Output MIDI devices get initialized.
	2) Call getOutputPorts() and getInputPorts() to retreive a map of portNumber/portName.
	3) Call openOutputPort(port) and/or openInputPort(port) to open an Output/Input port.
	4) Send MIDI events (for midi output) using send(QVector<unsigned char>).
	
	\todo Callback function implementation for retreiving MIDI-IN events. This should
	      probably be done by using Qt's signal-slot implementation. -Matevz
*/ 

CARtMidiDevice::CARtMidiDevice() {
	_midiDeviceType = RtMidiDevice;
	_out = 0;
	_in = 0;
	_outOpen=false;
	_inOpen=false;
	
	try {
		_out = new RtMidiOut();
		_in = new RtMidiIn();
	}
	catch (RtError &error) {
		error.printMessage();
	}
}

bool CARtMidiDevice::openOutputPort(int port) {
	if (port==-1)
		return false;
	
	if (_out && _out->getPortCount() > port) {	// check outputs
		try {
			_out->openPort(port);
		} catch (RtError &error) {
			error.printMessage();
			return false;	// error when opening the port
		}
		_outOpen=true;
		return true;	// port opened successfully
	} else {
		std::cerr << "CARtMidiDevice::openOutputPort(): Port number " << port << " doesn't exist!" << std::endl;
		return false;	// wrong port number specified
	}
}

bool CARtMidiDevice::openInputPort(int port) {
	if (port==-1)
		return false;
	
	if (_in && _in->getPortCount() > port) {	// check outputs
		try {
			_in->openPort(port);
		} catch (RtError &error) {
			error.printMessage();
			return false;	// error when opening the port
		}
		//_in->setCallback( &midiInCallback ); // sets the callback function \todo Compile errors :( -M
		_inOpen=true;
		return true;	// port opened successfully
	} else {
		std::cerr << "CARtMidiDevice::openInputPort(): Port number " << port << " doesn't exist!" << std::endl;
		return false;	// wrong port number specified
	}
}

/*!
	Callback function which gets called by RtMidi automatically when an information on MidiIn device has come.
*/
void CARtMidiDevice::midiInCallback( double deltatime, std::vector< unsigned char > *message, void *userData ) {
	emit midiInEvent( QVector<unsigned char>::fromStdVector(*message) );
}

void CARtMidiDevice::closeOutputPort() {
	try {
		if (_outOpen)
			_out->closePort();
	} catch (RtError &error) {
		error.printMessage();
	}
	_outOpen=false;
}

void CARtMidiDevice::closeInputPort() {
	try {
		if (_inOpen)
			_in->closePort();
	} catch (RtError &error) {
		error.printMessage();
	}
	_inOpen=false;
}

QMap<int, QString> CARtMidiDevice::getOutputPorts() {
	QMap<int, QString> outPorts;
	try {
		for (int i=0; _out && i<_out->getPortCount(); i++)
			outPorts.insert(i, QString::fromStdString(_out->getPortName(i)));
	} catch (RtError &error) {
		error.printMessage();
	}
	
	return outPorts;
}

QMap<int, QString> CARtMidiDevice::getInputPorts() {
	QMap<int, QString> inPorts;
	try {
		for (int i=0; _in && i<_in->getPortCount(); i++)
			inPorts.insert(i, QString::fromStdString(_in->getPortName(i)));
	} catch (RtError &error) {
		error.printMessage();
	}
	
	return inPorts;
}

CARtMidiDevice::~CARtMidiDevice() {
	closeOutputPort();
	closeInputPort();
	if (_out)
		delete _out;
	if (_in)
		delete _in;
}

void CARtMidiDevice::send(QVector<unsigned char> message) {
	std::vector<unsigned char> messageVector = message.toStdVector();
	if (_outOpen)
		_out->sendMessage(&messageVector);
}
