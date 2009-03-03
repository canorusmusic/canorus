/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef PLAYBACK_H_
#define PLAYBACK_H_

#include <QThread>
#include <QList>

class CAMidiDevice;
class CASheet;
class CAMusElement;
class CAPlayable;
class CANote;

class CAPlayback : public QThread {
#ifndef SWIG
Q_OBJECT
#endif
public:
	CAPlayback( CASheet*, CAMidiDevice* );
	CAPlayback( CAMidiDevice*, int port );
	~CAPlayback();

	void run();
	void stop();

	void playImmediately( QList<CAMusElement*> elts, int port );

	inline const int getInitTimeStart() { return _initTimeStart; }
	inline void setInitTimeStart(int t) { _initTimeStart = t; }
	inline CAMidiDevice *midiDevice() { return _midiDevice; }
	inline CASheet *sheet() { return _sheet; }
	inline void setSheet( CASheet *s ) { _sheet = s; }
	inline QList<CAPlayable*>& curPlaying() { return _curPlaying; }

#ifndef SWIG
public slots:
#else
public:
#endif
	void stopNow();

#ifndef SWIG
signals:
	void playbackFinished();
#endif

private:
	void initStreams( CASheet *sheet );
	void loopUntilPlayable( int i, bool ignoreRepeats=false );
	void playSelectionImpl();

	inline QList<CAMusElement*> streamAt(int idx) { return _stream[idx]; }
	inline int streamCount() { return _stream.size(); }
	inline int& streamIdx( int i ) { return _streamIdx[i]; }
	inline int& curTime( int i ) { return _curTime[i]; }
	inline int& lastRepeatOpenIdx( int i ) { return _lastRepeatOpenIdx[i]; }
	inline bool& repeating( int i ) { return _repeating[i]; }

	inline bool stopLock() { return _stopLock; }
	inline void setStopLock(bool lock) { _stopLock = lock; }

	CASheet *_sheet;

	CAMidiDevice *_midiDevice;
	inline void setMidiDevice( CAMidiDevice *d ) { _midiDevice = d; }

	inline void setStop(bool stop) { _stop = stop; }
	bool _stop;
	bool _stopLock;

	bool _playSelectionOnly;
	QList<CAMusElement*> _selection;

	int _initTimeStart;

	QList< QList<CAMusElement*> > _stream;
	QList<CAPlayable*> _curPlaying;	// list of currently playing notes and rests
	int *_streamIdx;
	bool *_repeating;
	int *_lastRepeatOpenIdx;
	int *_curTime; // multiple curTimes are needed for repeat bars, if staffs aren't synchronized
};

#endif /* PLAYBACK_H_ */
