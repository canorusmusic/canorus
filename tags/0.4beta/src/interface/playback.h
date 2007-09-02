/*! 
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef PLAYBACK_H_
#define PLAYBACK_H_

#include <QThread>
#include <QList>

class CAScoreViewPort;
class CAMidiDevice;
class CASheet;
class CAMusElement;
class CAPlayable;

class CAPlayback : public QThread {
public:
	CAPlayback(CAScoreViewPort*, CAMidiDevice *);
	CAPlayback(CASheet*, CAMidiDevice *);
	~CAPlayback();
	
	void run();
	void stop();
	
	inline const int getInitTimeStart() { return _initTimeStart; }
	inline void setInitTimeStart(int t) { _initTimeStart = t; }
	inline CAMidiDevice *midiDevice() { return _midiDevice; }
	inline CAScoreViewPort *scoreViewPort() { return _scoreViewPort; }
	inline CASheet *sheet() { return _sheet; }
	inline void setSheet( CASheet *s ) { _sheet = s; }
	inline QList<CAPlayable*>& curPlaying() { return _curPlaying; }
	
private:
	void initStreams( CASheet *sheet );
	void loopUntilPlayable( int i, bool ignoreRepeats=false );
	
	inline QList<CAMusElement*> streamAt(int idx) { return _stream[idx]; }
	inline int streamCount() { return _stream.size(); }
	inline int& streamIdx( int i ) { return _streamIdx[i]; }
	inline int& curTime( int i ) { return _curTime[i]; }
	inline int& lastRepeatOpenIdx( int i ) { return _lastRepeatOpenIdx[i]; }
	inline bool& repeating( int i ) { return _repeating[i]; }
	
	CAScoreViewPort *_scoreViewPort;
	inline void setScoreViewPort( CAScoreViewPort *v ) { _scoreViewPort = v; }
	
	CASheet *_sheet;
	
	CAMidiDevice *_midiDevice;
	inline void setMidiDevice( CAMidiDevice *d ) { _midiDevice = d; }
	
	inline void setStop(bool stop) { _stop = stop; }
	bool _stop;
	
	int _initTimeStart;
	
	QList< QList<CAMusElement*> > _stream;
	QList<CAPlayable*> _curPlaying;	// list of currently playing notes and rests
	int *_streamIdx;
	bool *_repeating;
	int *_lastRepeatOpenIdx;
	int *_curTime;
};

#endif /* PLAYBACK_H_ */
