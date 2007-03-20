/** @file interface/rtmididevice.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */
 
#ifndef RTMIDIDEVICE_H_
#define RTMIDIDEVICE_H_

#include "interface/mididevice.h"

class RtMidiOut;
class RtMidiIn;

/**
 * CARtMidiDevice is a Canorus wrapper class for a cross-platform MIDI library
 * RtMidi written by Gary P. Scavone (http://www.music.mcgill.ca/~gary/rtmidi/).
 * 
 * Usage:
 * 1) When created, Input and Output MIDI devices get initialized.
 * 2) Call getOutputPorts() and getInputPorts() to retreive a map of portNumber/portName.
 * 3) Call openOutputPort(port) and/or openInputPort(port) to open an Output/Input port.
 * 4) Send MIDI events (for midi output) using send(QVector<unsigned char>).
 * 
 * TODO: Callback function implementation for retreiving MIDI-IN events. This should
 *       probably be done by using Qt's signal-slot implementation. -Matevz
 */ 
class CARtMidiDevice : public CAMidiDevice {
	public:
		CARtMidiDevice();
		~CARtMidiDevice();
		
		QMap<int, QString> getOutputPorts();
		QMap<int, QString> getInputPorts();
		
		bool openOutputPort(int port);	// return true on success, false otherwise
		bool openInputPort(int port);	// return true on success, false otherwise	
		void closeOutputPort();
		void closeInputPort();
		void send(QVector<unsigned char> message);
		
	private:
		RtMidiOut *_out;
		RtMidiIn *_in;
		bool _outOpen;
		bool _inOpen;
};

#endif /*RTMIDIDEVICE_H_*/
