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
#include "score/sheet.h"
#include "score/staff.h"
#include "score/context.h"
#include "score/barline.h"
#include "score/note.h"
#include "score/voice.h"
#include "score/mark.h"
#include "score/dynamic.h"
#include "score/instrumentchange.h"
#include "score/tempo.h"
#include "score/timesignature.h"
#include "score/keysignature.h"

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
	6) Playback is also used for creating the events for midi file export. Therefore the music length time _curTime
	   is also transferred as a paramter in send() and sendMetaEvent() to export the music lengths independent of tempo.

	The playbackFinished() signal is emitted once playback has finished or stopped.

	If you want to immediately play only given elements (eg. when inserting notes), call playImmediately().
*/

CAPlayback::CAPlayback( CASheet *s, CAMidiDevice *m ) {
	initPlayback();

	_sheet = s;
	_midiDevice = m;
	_playSelectionOnly = false;
}

/*!
	Plays the list of music elements simultaniously.
	It only takes notes into account.
*/
CAPlayback::CAPlayback( CAMidiDevice *m ) {
	initPlayback();

	_midiDevice = m;
	_playSelectionOnly = true;
}

/*!
	Initializes all basic playback values to zero and connects the signals.
	Usually called only once from the constructor.
*/
void CAPlayback::initPlayback() {
	_repeating=0;
	_lastRepeatOpenIdx=0;
	_curTime=0;
	_streamIdx=0;
	_stop = false;
	_stopLock = false;

	// override this settings in actual constructor
	_sheet = 0;
	_midiDevice = 0;
	_playSelectionOnly = false;
	_initTimeStart = 0;
	_sleepFactor = 1.0; // set by tempo to determine the miliseconds for sleep

	connect(this, SIGNAL(finished()), SLOT(stopNow()));
}

/*!
	Destructor deletes the created arrays.
*/
CAPlayback::~CAPlayback() {
	if(isRunning())	{
		terminate();
		wait();
	}

	if (_lastRepeatOpenIdx)
		delete [] _lastRepeatOpenIdx;

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
	if ( !streamList().size() ) {
		initStreams( sheet() );
	}

	if ( !streamList().size() )
		stop();
	else
		setStop(false);

	int minLength = -1;
	int mSeconds=0;           // actual song time, used when creating a midi file
	while (!_stop || _curPlaying.size()) {	// at stop true: enter to switch all notes off
		for (int i=0; i<_curPlaying.size(); i++) {
			if ( _stop || _curPlaying[i]->timeEnd() <= _curTime ) {
				// note off
				CANote *note = dynamic_cast<CANote*>(_curPlaying[i]);
				if (note) {
					message << (128 + note->voice()->midiChannel()); // note off
					message << ( CADiatonicPitch::diatonicPitchToMidiPitch(note->diatonicPitch()) );
					message << (127);
					if ((note->musElementType()!=CAMusElement::Rest ) &&		// first because rest has no tie
							!(note->tieStart() && note->tieStart()->noteEnd()) )
						midiDevice()->send(message, _curTime);
					message.clear();
				}
				_curPlaying.removeAt(i--);
			}
		}

		for (int i=0; i<streamList().size(); i++) {
			loopUntilPlayable(i);
		}

		if (_stop) continue;	// no notes on anymore

		minLength = -1;
		for (int i=0; i<streamList().size(); i++) {

			while ( streamAt(i).size() > streamIdx(i) &&
			        streamAt(i).at(streamIdx(i))->timeStart() == _curTime
			      ) {

				// check if a rest carries a tempo mark
				CAMusElement *me = streamAt(i).at(streamIdx(i));
				if (me->musElementType()==CAMusElement::Rest) {
				    for (int j=0; j<me->markList().size(); j++) {
				    	if ( me->markList()[j]->markType()==CAMark::Tempo ) {
				    		CATempo *tempo = static_cast<CATempo*>(me->markList()[j]);
		    				midiDevice()->sendMetaEvent(_curTime, CAMidiDevice::Meta_Tempo, tempo->bpm(), 0, 0);
						}
					}

				}

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
				    		midiDevice()->send(message, _curTime);
				    		message.clear();
				    	} else
				    	if ( note->markList()[j]->markType()==CAMark::InstrumentChange ) {
							message << (192 + note->voice()->midiChannel()); // change program
							message << static_cast<unsigned char>(static_cast<CAInstrumentChange*>(note->markList()[j])->instrument());
							midiDevice()->send(message, _curTime);
							message.clear();
				    	} else
				    	if ( note->markList()[j]->markType()==CAMark::Tempo ) {
				    		updateSleepFactor( static_cast<CATempo*>(note->markList()[j]) );
				    		CATempo *tempo = static_cast<CATempo*>(note->markList()[j]);
		    				midiDevice()->sendMetaEvent(_curTime, CAMidiDevice::Meta_Tempo, tempo->bpm(), 0, 0);
				    	}
				    }

					message << (144 + note->voice()->midiChannel()); // note on
					message << ( CADiatonicPitch::diatonicPitchToMidiPitch(note->diatonicPitch()) );
					message << (127);
					if ( !note->tieEnd() )
						midiDevice()->send(message, _curTime);
					message.clear();
				}

				if (streamAt(i).at(streamIdx(i))->isPlayable()) {
					_curPlaying << static_cast<CAPlayable*>(streamAt(i).at(streamIdx(i)));
				}

				int delta;
				if ( (delta = (streamAt(i).at(streamIdx(i))->timeEnd() - _curTime)) < minLength
				    ||
				     minLength==-1
				   )
					minLength = delta;

				streamIdx(i)++;
			}

			// calculate the pause needed by msleep
			// last playables in the stream - _curPlaying is otherwise always set!
			// pre-last pass, set minLength to their timeLengths to stop the notes
			for (int j=0; j<_curPlaying.size(); j++) {
				if ((_curPlaying[j]->timeEnd() - _curTime) < minLength || minLength==-1)
					minLength =_curPlaying[j]->timeEnd() - _curTime;
			}
		}

		if (minLength==-1) {
			// last pass, notes indices are at the ends and no notes are played anymore
			setStop(true);
		}

		if (minLength!=-1) {
			mSeconds += qRound(minLength*_sleepFactor);

			if ( midiDevice()->isRealTime() )
				msleep( qRound(minLength*_sleepFactor) );

			_curTime += minLength;
		}
	}

	_curPlaying.clear();
	stop();
}

/*!
	Calculates the sleep factor for the given tempo \a t.
	If \a t is null, it does nothing.
 */
void CAPlayback::updateSleepFactor( CATempo *t ) {
	if (t) {
		_sleepFactor = 60000.0 /
		              ( CAPlayableLength::playableLengthToTimeLength( t->beat() ) * t->bpm() );
	}
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
			midiDevice()->send(message, _curTime);
			message.clear();

			message << (176 + note->voice()->midiChannel()); // set volume
			message << (7);
			message << (100);
			midiDevice()->send(message, _curTime);
			message.clear();

			message << (144 + note->voice()->midiChannel()); // note on
			message << ( CADiatonicPitch::diatonicPitchToMidiPitch(note->diatonicPitch()) );
			message << (127);
			midiDevice()->send(message, _curTime);
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
					message << ( CADiatonicPitch::diatonicPitchToMidiPitch(note->diatonicPitch()) );
					message << (127);
					midiDevice()->send(message, _curTime);
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
	for (int i=0; i < sheet->contextList().size(); i++) {
		if (sheet->contextList()[i]->contextType() == CAContext::Staff) {
			CAStaff *staff = static_cast<CAStaff*>(sheet->contextList()[i]);
			// add all the voices lists to the common list stream
			for (int j=0; j < staff->voiceList().size(); j++) {
				_streamList << staff->voiceList()[j]->musElementList();

				QVector<unsigned char> message;
				message << (192 + staff->voiceList()[j]->midiChannel()); // change program
				message << (staff->voiceList()[j]->midiProgram());
				midiDevice()->send(message, _curTime);
				message.clear();

				message << (176 + staff->voiceList()[j]->midiChannel()); // set volume
				message << (7);
				message << (100);
				midiDevice()->send(message, _curTime);
				message.clear();
			}
		}
	}
	_streamIdx = new int[streamList().size()];
	_lastRepeatOpenIdx = new int[streamList().size()];

	// init streams indices, current times and last repeat barlines
	for (int i=0; i<streamList().size(); i++) {
		_curTime = getInitTimeStart();
		streamIdx(i) = 0;
		lastRepeatOpenIdx(i) = -1;
		_repeating = false;
		loopUntilPlayable(i, true); // ignore repeats
	}

	if (_sheet) {
		updateSleepFactor( _sheet->getTempo(getInitTimeStart()) );
	}
}


/*!
	Loops from the stream with the given index \a i until the last element with smaller or equal start time of the current time.
	This function also remembers any special signs like open repeat barlines.
*/
void CAPlayback::loopUntilPlayable( int i, bool ignoreRepeats ) {
	for (int j=streamIdx(i);
	     j<streamAt(i).size() &&
	     streamAt(i).at(j)->timeStart() <= _curTime &&
	     (streamAt(i).at(j)->timeStart() != _curTime ||
	      !(streamAt(i).at(j)->musElementType()==CAMusElement::Note) ||
	      (static_cast<CANote*>(streamAt(i).at(j))->isFirstInChord())
	     );
	     streamIdx(i) = j++) {

		if ( streamAt(i).at(j)->musElementType()==CAMusElement::TimeSignature ) {

			int beats = static_cast<CATimeSignature*>(streamAt(i).at(j))->beats();
			int beat = static_cast<CATimeSignature*>(streamAt(i).at(j))->beat();
			//std::cout<<"  exportiere Time Signature    "<<_curTime<<" mit "<<beats<<"/"<<beat<<std::endl;
		    midiDevice()->sendMetaEvent( _curTime, CAMidiDevice::Meta_Timesig, beats, beat, 0 );
		}
		if ( streamAt(i).at(j)->musElementType()==CAMusElement::KeySignature ) {
			//int key = (static_cast<CAKeySignature*>(streamAt(i).at(j)))->diatonicKey()->numberOfAccs();
			CAKeySignature *ks = dynamic_cast<CAKeySignature*>(streamAt(i).at(j));
			CADiatonicKey dk = ks->diatonicKey();
			int key = dk.numberOfAccs();
			int minor = dk.gender() == CADiatonicKey::Minor ? 1 : 0;
		    midiDevice()->sendMetaEvent( _curTime, CAMidiDevice::Meta_Keysig, key, minor, 0 );
		}

		if ( streamAt(i).at(j)->musElementType()==CAMusElement::Barline &&
		     static_cast<CABarline*>(streamAt(i).at(j))->barlineType()==CABarline::RepeatOpen
		   ) {
			lastRepeatOpenIdx(i) = j;
		}

		if ( streamAt(i).at(j)->musElementType()==CAMusElement::Barline &&
		     static_cast<CABarline*>(streamAt(i).at(j))->barlineType()==CABarline::RepeatClose &&
		     !ignoreRepeats ) {
			if (!_repeating) {
				// set the new index in ALL streams
				for (int k=0; k<streamList().size(); k++) {
					streamIdx(k) = lastRepeatOpenIdx(k)+1;
				}

				_curTime = streamAt(i).at(streamIdx(i))->timeStart();
				j = streamIdx(i);
				_repeating = true;
			}
		}
	}

	// last element if non-playable is exception - increase the index counter
	if (streamIdx(i)==streamAt(i).size()-1 && !streamAt(i).at(streamIdx(i))->isPlayable())
		streamIdx(i)++;
}
