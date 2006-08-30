/** @file mididevice.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef MIDIDEVICE_H_
#define MIDIDEVICE_H_

#include <vector>

class CAMidiDevice {
	public:
		enum CAMidiDeviceType {
			RtMidiDevice
		};
		
		CAMidiDeviceType midiDeviceType() { return _midiDeviceType; };
		
		virtual void send(std::vector<unsigned char>* message) = 0;
		
	private:
		CAMidiDeviceType _midiDeviceType;
};

#endif /*MIDIDEVICE_H_*/
