/** @file playback.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef PLAYBACK_H_
#define PLAYBACK_H_

#include <QThread>

class CAScoreViewPort;
class CAMidiDevice;

class CAPlayback : public QThread {
	Q_OBJECT
	public:
		CAPlayback(CAScoreViewPort *v, CAMidiDevice *m);
		void run();
		void stop();
	
	private:
		CAScoreViewPort *_scoreViewPort;
		bool _stop;
		CAMidiDevice *_midiDevice;
};

#endif /*PLAYBACK_H_*/
