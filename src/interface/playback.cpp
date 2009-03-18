/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPen>
#include <QRect>
#include <QVector>	// needed for RtMidi send message

#include <iostream>

#include "interface/playback.h"
#include "interface/mididevice.h"
#include "core/sheet.h"
#include "core/staff.h"
#include "core/context.h"
#include "core/barline.h"
#include "core/note.h"
#include "core/voice.h"
#include "core/mark.h"
#include "core/dynamic.h"
#include "core/instrumentchange.h"
#include "core/tempo.h"

/*!
	\class CAPlayback
	\brief Audio playback of the score.
	This class creates playback events (usually MIDI events) for the music elements and sends these events to
	one of the playback devices (usually CAMidiDevice).

	To use the playback capabilities:
	1) Create one of the CAMidiDevices (eg. CARtMidiDevice) and configure it (open output/input port).
	2) Create CAPlayback object passing it the current score viewport (played notes will be painted red) or only
	   the sheet (usually used in scripting environment) and the midi device.
	3) Optionally configure playback (setInitTimeStart() to start playback from the specific time. Default 0).
	4) Call myPlaybackObject->run(). This will start playing in a new thread.
	5) Call myPlaybackObject->stop() to stop the playback. Playback also stops automatically when finished.

	The playbackFinished() signal is emitted once playback has finished or stopped.

	If you want to immediately play only given elements (eg. when inserting notes), call playImmediately().
*/

CAPlayback::CAPlayback( CASheet *s, CAMidiDevice *m ) {
	setSheet( s );
	setMidiDevice( m );
	_stop = false;
	setInitTimeStart( 0 );
	setStopLock(false);
	connect(this, SIGNAL(finished()), SLOT(stopNow()));
	_playSelectionOnly = false;

	_repeating=0;
	_lastRepeatOpenIdx=0;
	_curTime=0;
	_streamIdx=0;
}

/*!
	Plays the list of music elements simultaniously.
	It only takes notes into account.
*/
CAPlayback::CAPlayback( CAMidiDevice *m, int port ) {
	setMidiDevice( m );
	_stop = false;
	setStopLock(false);
	_playSelectionOnly = true;
	connect(this, SIGNAL(finished()), SLOT(stopNow()));

	_repeating=0;
	_lastRepeatOpenIdx=0;
	_curTime=0;
	_streamIdx=0;
}

/*!
	Destructor deletes the created arrays.
*/
CAPlayback::~CAPlayback() {
	if(isRunning())	{
		terminate();
		wait();
	}

	if (_repeating)
		delete [] _repeating;

	if (_lastRepeatOpenIdx)
		delete [] _lastRepeatOpenIdx;

	if (_curTime)
		delete [] _curTime;

	if (_streamIdx)
		delete [] _streamIdx;
}

/*!
	Immediately plays the given \a elts.
 */
void CAPlayback::playImmediately( QList<CAMusElement*> elts, int port ) {
	_selection << elts;

	midiDevice()->openOutputPort( port );

	if (!isRunning()) {
		start();
	}
}

void CAPlayback::run() {
	if ( _playSelectionOnly ) {
		playSelectionImpl();
		return;
	}

	// list of all the music element lists (ie. streams) taken from all the contexts
	QList< QList<CAMusElement*> > stream;
	QVector<unsigned char> message;	// midi 3-byte message sent to midi device

	// initializes all the streams, indices, repeat barlines etc.
	if ( !streamCount() )
		initStreams( sheet() );

	if ( !streamCount() )
		stop();
	else
		setStop(false);

	int minLength = -1;
	float sleepFactor = 1.0;  // set by tempo to determine the miliseconds for sleep
	int mSeconds=0;           // actual song time, used when creating a midi file
	while (!_stop || _curPlaying.size()) {	// at stop true: enter to switch all notes off
		for (int i=0; i<streamCount(); i++) {
			loopUntilPlayable(i);
		}

		for (int i=0; i<_curPlaying.size(); i++) {
			if ( _stop || _curPlaying[i]->timeEnd() <= curTime(i) ) {
				// note off
				CANote *note = dynamic_cast<CANote*>(_curPlaying[i]);
				if (note) {
					message << (128 + note->voice()->midiChannel()); // note off
					message << ( CAMidiDevice::diatonicPitchToMidiPitch(note->diatonicPitch()) );
					message << (127);
					if ((note->musElementType()!=CAMusElement::Rest ) &&		// first because rest has no tie
							!(note->tieStart() && note->tieStart()->noteEnd()) )
						midiDevice()->send(message, mSeconds);
					message.clear();
				}
				_curPlaying.removeAt(i--);
			}
		}

		if (_stop) continue;	// no notes on anymore

		minLength = -1;
		for (int i=0; i<streamCount(); i++) {
			while ( streamAt(i).size() > streamIdx(i) &&
			        streamAt(i).at(streamIdx(i))->timeStart() == curTime(i)
			      ) {
				// note on
				CANote *note = dynamic_cast<CANote*>(streamAt(i).at(streamIdx(i)));

				if (note) {
				    QVector<unsigned char> message;

				    // send dynamic information
				    for (int j=0; j<note->markList().size(); j++) {
				    	if ( note->markList()[j]->markType()==CAMark::Dynamic ) {
				    		message << (176 + note->voice()->midiChannel()); // set volume
				    		message << (CAMidiDevice::Midi_Ctl_Volume /* 7 */ );
				    		message << qRound(127 * static_cast<CADynamic*>(note->markList()[j])->volume()/100.0);
				    		midiDevice()->send(message, mSeconds);
				    		message.clear();
				    	} else
				    	if ( note->markList()[j]->markType()==CAMark::InstrumentChange ) {
							message << (192 + note->voice()->midiChannel()); // change program
							message << static_cast<unsigned char>(static_cast<CAInstrumentChange*>(note->markList()[j])->instrument());
							midiDevice()->send(message, mSeconds);
							message.clear();
				    	} else
				    	if ( note->markList()[j]->markType()==CAMark::Tempo ) {
				    		sleepFactor = 60000.0 /
				    		( CAPlayableLength::playableLengthToTimeLength( static_cast<CATempo*>(note->markList()[j])->beat() )
				    		  * static_cast<CATempo*>(note->markList()[j])->bpm()
				    		);
				    	}
				    }

					message << (144 + note->voice()->midiChannel()); // note on
					message << ( CAMidiDevice::diatonicPitchToMidiPitch(note->diatonicPitch()) );
					message << (127);
					if ( !note->tieEnd() )
						midiDevice()->send(message, mSeconds);
					message.clear();
				}

				if (streamAt(i).at(streamIdx(i))->isPlayable()) {
					_curPlaying << static_cast<CAPlayable*>(streamAt(i).at(streamIdx(i)));
				}

				int delta;
				if ( (delta = (streamAt(i).at(streamIdx(i))->timeEnd() - _curTime[i])) < minLength
				    ||
				     minLength==-1
				   )
					minLength = delta;

				streamIdx(i)++;
			}

			// calculate the pause needed by msleep
			// last playables in the stream - _curPlaying is otherwise always set!
			// pre-last pass, set minLength to their timeLengths to stop the notes
			// \todo curtime() below doesn't work yet for asynchrone staffs (not-aligned repeat bars)
			for (int j=0; j<_curPlaying.size(); j++) {
				if ((_curPlaying[j]->timeEnd() - curTime(i)) < minLength || minLength==-1)
					minLength =_curPlaying[j]->timeEnd() - curTime(i);
			}
		}

		if (minLength==-1) {
			// last pass, notes indices are at the ends and no notes are played anymore
			setStop(true);
		}

		if (minLength!=-1) {
			mSeconds += qRound(minLength*sleepFactor);

			if ( midiDevice()->isRealTime() )
				msleep( qRound(minLength*sleepFactor) );

			for (int i=0; i<streamCount(); i++)
				curTime(i) += minLength;
		}
	}

	_curPlaying.clear();
	stop();
}

/*!
	Private function for immediately playing the music elements in _selection.
	This function ends when all the notes in _selection queue are played.
	_selection queue might be refilled during the playback by calling playImmediately().

	This function is usually called when inserting new notes. If the first note is still playing,
	the second note is played simultaniously - thus having two notes in _selection for eg.
 */
void CAPlayback::playSelectionImpl() {
	QVector<unsigned char> message;
	QList<int> timeEnds;       // time ends when the notes should turned off
	int waitTime = 16;
	int curTime = 0;

	while (_selection.size() || _curPlaying.size()) {
		while (_selection.size()) {
			if ( _selection[0]->musElementType()!=CAMusElement::Note ) {
				_selection.takeFirst();
				continue;
			}

			CANote *note = static_cast<CANote*>(_selection.takeFirst());

			// Note ON
			message << (192 + note->voice()->midiChannel()); // change program
			message << (note->voice()->midiProgram());
			midiDevice()->send(message, 0);
			message.clear();

			message << (176 + note->voice()->midiChannel()); // set volume
			message << (7);
			message << (100);
			midiDevice()->send(message, 0);
			message.clear();

			message << (144 + note->voice()->midiChannel()); // note on
			message << ( CAMidiDevice::diatonicPitchToMidiPitch(note->diatonicPitch()) );
			message << (127);
			midiDevice()->send(message, 0);
			message.clear();

			_curPlaying << note;
			timeEnds << curTime + note->timeLength()*4;
		}

		for (int i=0; i<_curPlaying.size(); i++) {
			if (curTime >= timeEnds[i] || _stop) {
				// Note OFF
				if (_curPlaying[i]->musElementType()==CAMusElement::Note) {
					CANote *note = static_cast<CANote*>(_curPlaying[i]);

					message << (128 + note->voice()->midiChannel()); // note off
					message << ( CAMidiDevice::diatonicPitchToMidiPitch(note->diatonicPitch()) );
					message << (127);
					midiDevice()->send(message, 0);
					message.clear();
				}

				timeEnds.removeAt(i);
				_curPlaying.removeAt(i);
				i--;
			}
		}

		msleep( waitTime );
		curTime += waitTime;
	}

	stop();
	// output ports are closed in MainWin
}

/*!
	The nice and the right way to stop the playback.
	Returns immediately.

	\sa stopNow()
*/
void CAPlayback::stop() {
	_stop = true;
}

/*!
	Stop playback and clean up.
	Blocks until all cleanups are done.

	\sa stop()
*/
void CAPlayback::stopNow()
{
	if(stopLock())
		return;
	setStopLock(true);
	if(isRunning()) { // stopNow() was _not_ called by the finished() signal (i.e. it was called from another thread)
		// stop playback and wait for the thread to finish.
		stop();
		wait(); // (QThread::finished() will be emitted here, so we need the lock flag)
	}
	setStopLock(false);
	emit playbackFinished();
}

/*!
	Generates streams (elements lists) of playable elements (notes, rests) from the given sheet.
*/
void CAPlayback::initStreams( CASheet *sheet ) {
	for (int i=0; i < sheet->contextCount(); i++) {
		if (sheet->contextAt(i)->contextType() == CAContext::Staff) {
			CAStaff *staff = static_cast<CAStaff*>(sheet->contextAt(i));
			// add all the voices lists to the common list stream
			for (int j=0; j < staff->voiceCount(); j++) {
				_stream << staff->voiceAt(j)->musElementList();

				QVector<unsigned char> message;
				message << (192 + staff->voiceAt(j)->midiChannel()); // change program
				message << (staff->voiceAt(j)->midiProgram());
				midiDevice()->send(message, 0);
				message.clear();

				message << (176 + staff->voiceAt(j)->midiChannel()); // set volume
				message << (7);
				message << (100);
				midiDevice()->send(message, 0);
				message.clear();
			}
		}
	}
	_streamIdx = new int[streamCount()];
	_curTime = new int[streamCount()];
	_lastRepeatOpenIdx = new int[streamCount()];
	_repeating = new bool[streamCount()];

	// init streams indices, current times and last repeat barlines
	for (int i=0; i<streamCount(); i++) {
		curTime(i) = getInitTimeStart();
		streamIdx(i) = 0;
		lastRepeatOpenIdx(i) = -1;
		repeating(i) = false;
		loopUntilPlayable(i, true); // ignore repeats
	}
}

/*!
	Loops from the stream with the given index \a i until the last element with smaller or equal start time of the current time.
	This function also remembers any special signs like open repeat barlines.
*/
void CAPlayback::loopUntilPlayable( int i, bool ignoreRepeats ) {
	for (int j=streamIdx(i);
	     j<streamAt(i).size() &&
	     streamAt(i).at(j)->timeStart() <= curTime(i) &&
	     (streamAt(i).at(j)->timeStart() != curTime(i) ||
	      !(streamAt(i).at(j)->musElementType()==CAMusElement::Note) ||
	      (static_cast<CANote*>(streamAt(i).at(j))->isFirstInChord())
	     );
	     streamIdx(i) = j++) {
		if ( streamAt(i).at(j)->musElementType()==CAMusElement::Barline &&
		     static_cast<CABarline*>(streamAt(i).at(j))->barlineType()==CABarline::RepeatOpen
		   ) {
			lastRepeatOpenIdx(i) = j;
		}

		if ( streamAt(i).at(j)->musElementType()==CAMusElement::Barline &&
		     static_cast<CABarline*>(streamAt(i).at(j))->barlineType()==CABarline::RepeatClose &&
		     !ignoreRepeats ) {
			if (repeating(i))
				repeating(i) = false;
			else {
				j = streamIdx(i) = lastRepeatOpenIdx(i)+1;
				curTime(i) = streamAt(i).at(streamIdx(i))->timeStart();
				repeating(i) = true;
			}
		}
	}

	// last element if non-playable is exception - increase the index counter
	if (streamIdx(i)==streamAt(i).size()-1 && !streamAt(i).at(streamIdx(i))->isPlayable())
		streamIdx(i)++;
}
