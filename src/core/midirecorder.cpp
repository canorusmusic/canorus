/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/midirecorder.h"
#include "export/midiexport.h"
#include "core/resource.h"
#include "interface/mididevice.h"

/*!
	\class CAMidiRecorder
	\brief Class for live recording of the Midi events

	This class is used when a composer has a musical idea in his fingers and
	wants to record it.

	1) Create a new output resource (eg. midi file in tmp directory)
	2) Create this class and pass this resource
	2) Call record(). Class will run in a separated thread and start recording
	   all the midi events into the given resource file.
	3) Call stop() when recording is done. Class will write the midi data and
	   close the stream.
 */
CAMidiRecorder::CAMidiRecorder( CAResource *r, CAMidiDevice *d )
 : QObject(), _resource(r), _curTime(0), _lastTime(0), _midiExport(0) {
	_timer = new QTimer();
	_timer->setInterval(10);
	connect( _timer, SIGNAL(timeout()), this, SLOT(timerTimeout()) );

	connect( d, SIGNAL(midiInEvent( QVector<unsigned char> )), this, SLOT(onMidiInEvent( QVector<unsigned char> )) );
}

CAMidiRecorder::~CAMidiRecorder() {
	disconnect();
}

void CAMidiRecorder::timerTimeout() {
	_curTime+=10;
}

void CAMidiRecorder::startRecording( int startTime ) {
	_midiExport = new CAMidiExport();
	_midiExport->setStreamToFile( _resource->url().toLocalFile() );

	_timer->start();
}

void CAMidiRecorder::stopRecording() {
	_midiExport->writeFile();

	delete _midiExport;
	_midiExport = 0;
}

void CAMidiRecorder::onMidiInEvent( QVector<unsigned char> messages ) {
	if (_midiExport) {
		_midiExport->send( messages, deltaTime() );
	}
}

/*!
	Returns the difference in mseconds between the current time and
	the last time accessed by this function.
 */
const unsigned int CAMidiRecorder::deltaTime() {
	unsigned int delta = _curTime - _lastTime;
	_lastTime = _curTime;

	return delta;
}
