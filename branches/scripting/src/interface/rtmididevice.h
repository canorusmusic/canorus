/** @file rtmididevice.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */
 
#ifndef RTMIDIDEVICE_H_
#define RTMIDIDEVICE_H_

#include "mididevice.h"

class RtMidiOut;
class RtMidiIn;

class CARtMidiDevice : public CAMidiDevice {
	public:
		CARtMidiDevice();
		~CARtMidiDevice();
		void send(std::vector<unsigned char>* message);
		void openPort();
		void closePort();
	
	private:
		RtMidiOut *_out;
		RtMidiIn *_in;
};

#endif /*RTMIDIDEVICE_H_*/
