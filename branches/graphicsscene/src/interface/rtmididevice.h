/*!
	Copyright (c) 2006, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/
 
#ifndef RTMIDIDEVICE_H_
#define RTMIDIDEVICE_H_

#include "interface/mididevice.h"
#include <sstream>

class RtMidiOut;
class RtMidiIn;

#ifndef SWIG
void rtMidiInCallback( double deltatime, std::vector< unsigned char > *message, void *userData );
#endif

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
	void send(QVector<unsigned char> message, int time);
	void sendMetaEvent(int time, int event, int a, int b, int c ) { ; }
	
private:
	RtMidiOut *_out;
	RtMidiIn *_in;
	bool _outOpen;
	bool _inOpen;
	qint64 _pid;
	std::stringstream _midiNameIn;
	std::stringstream _midiNameOut;
};

#endif /* RTMIDIDEVICE_H_ */
