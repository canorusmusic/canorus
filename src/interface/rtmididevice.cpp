/*!
	Copyright (c) 2006-2020, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QCoreApplication>
#include <QVector>
#include <sstream>

#include "interface/rtmididevice.h"
#include "../lib/rtmidi-4.0.0/RtMidi.h"

#ifndef SWIGCPP
#include "canorus.h"
#endif

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

CARtMidiDevice::CARtMidiDevice()
 : CAMidiDevice() {
	_midiDeviceType = RtMidiDevice;
	_out = nullptr;
	_in = nullptr;
	_outOpen=false;
	_inOpen=false;
	setRealTime(true);

	// create midi client names which hold the current pid
	_pid = QCoreApplication::applicationPid();
	_midiNameOut << "Canorus Out (" << _pid << ")";
	_midiNameIn << "Canorus In (" << _pid << ")";

	try {
        _out = new RtMidiOut( RtMidi::UNSPECIFIED, _midiNameOut.str() );
        _in = new RtMidiIn( RtMidi::UNSPECIFIED, _midiNameIn.str() );
	}
    catch (RtMidiError &error) {
		error.printMessage();
	}
}

bool CARtMidiDevice::openOutputPort(int port) {
	if (port==-1 || _outOpen)
		return false;

	if (_out && static_cast<int>(_out->getPortCount()) > port) {	// check outputs
		try {
			_out->openPort(static_cast<unsigned int>(port));
        } catch (RtMidiError &error) {
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
	if (port==-1 || _inOpen)
		return false;

	if (_in && static_cast<int>(_in->getPortCount()) > port) {	// check outputs
		try {
			_in->openPort(static_cast<unsigned int>(port));
        } catch (RtMidiError &error) {
			error.printMessage();
			return false;	// error when opening the port
		}
		_in->setCallback( &rtMidiInCallback ); // sets the callback function
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
void rtMidiInCallback( double, std::vector< unsigned char > *message, void * )
{
    (void)message; // Only used in with SWIGCPP
#ifndef SWIGCPP
	emit CACanorus::midiDevice()->midiInEvent( QVector< unsigned char >::fromStdVector(*message) );
#else
	// call scripting callback?
#endif
}

void CARtMidiDevice::closeOutputPort() {
	try {
		if (_outOpen)
			_out->closePort();
    } catch (RtMidiError &error) {
		error.printMessage();
	}
	_outOpen=false;
}

void CARtMidiDevice::closeInputPort() {
	try {
		if (_inOpen) {
			_in->cancelCallback();
			_in->closePort();
		}
    } catch (RtMidiError &error) {
		error.printMessage();
	}
	_inOpen=false;
}

QMap<int, QString> CARtMidiDevice::getOutputPorts() {
	QMap<int, QString> outPorts;
	try {
		for (int i=0; _out && i<static_cast<int>(_out->getPortCount()); i++)
			outPorts.insert(i, QString::fromStdString(_out->getPortName(static_cast<unsigned int>(i))));
    } catch (RtMidiError &error) {
		error.printMessage();
	}

	return outPorts;
}

QMap<int, QString> CARtMidiDevice::getInputPorts() {
	QMap<int, QString> inPorts;
	try {
		for (int i=0; _in && i<static_cast<int>(_in->getPortCount()); i++)
			inPorts.insert(i, QString::fromStdString(_in->getPortName(static_cast<unsigned int>(i))));
    } catch (RtMidiError &error) {
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

/*!
	Sends the given \a message to the midi device. \a offset is ignored because CARtMidiDevice is a realtime device.
*/
void CARtMidiDevice::send(QVector<unsigned char> message, int) {
	std::vector<unsigned char> messageVector = message.toStdVector();
	if (_outOpen)
		_out->sendMessage(&messageVector);
}
