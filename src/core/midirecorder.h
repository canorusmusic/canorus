/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef MIDIRECORDER_H_
#define MIDIRECORDER_H_

#include <QTimer>
#include <QVector>

class CAMidiExport;
class CAResource;
class CAMidiDevice;

class CAMidiRecorder : public QObject {
#ifndef SWIG
	Q_OBJECT
#endif
public:
	CAMidiRecorder( CAResource *r, CAMidiDevice *d );
	virtual ~CAMidiRecorder();

	void startRecording( int time=0 );
	void pauseRecording();
	void stopRecording();

	const unsigned int& curTime() const { return _curTime; }

private slots:
	void timerTimeout();
	void onMidiInEvent( QVector<unsigned char> messages );

private:
	CAResource   *_resource;
	CAMidiExport *_midiExport;
	QTimer       *_timer;
	unsigned int  _curTime;

	bool          _paused;
};

#endif /* MIDIRECORDER_H_ */
