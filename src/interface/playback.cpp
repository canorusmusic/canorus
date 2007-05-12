/*!
	Copyright (c) 2006, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPen>
#include <QRect>
#include <QVector>	// needed for RtMidi send message

#include <iostream>

#include "interface/playback.h"
#include "interface/mididevice.h"
#include "widgets/scoreviewport.h"
#include "core/sheet.h"
#include "core/staff.h"
#include "core/context.h"
#include "core/barline.h"
#include "core/note.h"
#include "drawable/drawablecontext.h"
#include "drawable/drawablemuselement.h"
#include "core/voice.h"

/*!
	\class CAPlayback
	\brief Audio playback of the score.
	This class creates playback events (usually MIDI events) for the music elements and sends these events to
	one of the playback devices (usually CAMidiDevice).
	
	To use the playback capabilities:
	1) Create CAPlayback object passing it the current score viewport (played notes will be painted red) and the midi device.
	2) Optionally configure it (setInitTimeStart() to start playback from the specific time. Default 0).
	3) Call myPlaybackObject->run(). This will start playing in a new thread.
	4) Call myPlaybackObject->stop() to stop the playback. Playback will also stops automatically when finished.
*/

CAPlayback::CAPlayback(CAScoreViewPort *v, CAMidiDevice *m) {
	setScoreViewPort( v );
	setMidiDevice( m );
	_stop = false;
	setInitTimeStart( 0 );
}

/*!
	Destructor deletes the created arrays.
*/
CAPlayback::~CAPlayback() {
	delete _streamIdx;
	delete _lastRepeatOpenIdx;
	delete _curTime;
}

void CAPlayback::run() {
	QPen p;
	p.setColor(Qt::green);
	p.setWidth(3);
	
	scoreViewPort()->setBorder(p);
	scoreViewPort()->setPlaying(true);	// set the deadlock for borders
	
	// list of all the music element lists (ie. streams) taken from all the contexts
	QList< QList<CAMusElement*> > stream; 
	QVector<unsigned char> message;	// midi 3-byte message sent to midi device

	QList<CADrawableMusElement *> oldSelection;
	oldSelection = scoreViewPort()->selection();
	scoreViewPort()->clearSelection();
	
	// initializes all the streams, indices, repeat barlines etc.
	initStreams( scoreViewPort()->sheet() );
	
	if ( !streamCount() )
		stop();
	else
		setStop(false);
	
	int minLength = -1;
	while (!_stop) {
		for (int i=0; i<streamCount(); i++) {
			loopUntilPlayable(i);
		}
		
		for (int i=0; i<_curPlaying.size(); i++) {
			if ( _curPlaying[i]->timeStart() + _curPlaying[i]->timeLength() >= curTime(i) ) {
				// note off
				CANote *note = dynamic_cast<CANote*>(_curPlaying[i]);
				if (note) {
					message << (128 + note->voice()->midiChannel()); // note off
					message << (note->midiPitch());
					message << (127);
					if ((note->musElementType()!=CAMusElement::Rest ) &&		// first because rest has no tie
							!(note->tieStart() && note->tieStart()->noteEnd()) )
						midiDevice()->send(message);
					message.clear();
				}
				scoreViewPort()->removeFromSelection( scoreViewPort()->findMElement(_curPlaying[i]) );
				//_scoreViewPort->setRepaintArea(new QRect(drawable->xPos(), drawable->yPos(), drawable->width(), drawable->height()));			      	
			    //_scoreViewPort->repaint();
				
				_curPlaying.removeAt(i--);				
			}
		}
		
		minLength = -1;
		for (int i=0; i<streamCount(); i++) {
			while ( streamAt(i).size() > streamIdx(i) &&
			        ( streamAt(i).at(streamIdx(i))->musElementType()==CAMusElement::Note ||
			          streamAt(i).at(streamIdx(i))->musElementType()==CAMusElement::Rest ) &&
			        streamAt(i).at(streamIdx(i))->timeStart() == curTime(i)
			      ) {
				// note on
				CANote *note = dynamic_cast<CANote*>(streamAt(i).at(streamIdx(i)));
				
				if (note) {
				    QVector<unsigned char> message;
					message << (144 + note->voice()->midiChannel()); // note on
					message << (note->midiPitch());
					message << (127);
					if ( note->musElementType()!=CAMusElement::Rest && !note->tieEnd() )
						midiDevice()->send(message);
					message.clear();
				}
				_curPlaying << static_cast<CAPlayable*>(streamAt(i).at(streamIdx(i)));
				
				scoreViewPort()->addToSelection( scoreViewPort()->findMElement( streamAt(i).at(streamIdx(i)) ) );

				//_scoreViewPort->setRepaintArea(new QRect(drawable->xPos(), drawable->yPos(), drawable->width(), drawable->height()));			      	
				//_scoreViewPort->repaint();
				
				int delta;
				if ( (delta = streamAt(i).at(streamIdx(i))->timeStart() + streamAt(i).at(streamIdx(i))->timeLength() - _curTime[i]) < minLength
				    ||
				     minLength==-1
				   )
					minLength = delta;
				
				streamIdx(i)++;
			}
			
			// calculate the pause needed by msleep
		}
		
		// last playables in the stream - _curPlaying is otherwise always set!
		// pre-last pass, set minLength to their timeLengths to stop the notes
		for (int i=0; i<_curPlaying.size(); i++) {
			if (_curPlaying[i]->timeLength() < minLength || minLength==-1)
				minLength = _curPlaying[i]->timeLength();			
		}
		
		if (minLength==-1) {
			// last pass, notes indices are at the ends and no notes are played anymore
			setStop(true);
		}
		
		if (minLength!=-1) {
			msleep(minLength);
			for (int i=0; i<streamCount(); i++)
				curTime(i) += minLength;
		}
	}
	
	
	scoreViewPort()->clearSelection();
	scoreViewPort()->addToSelection( oldSelection );
	scoreViewPort()->unsetBorder();
	stop();
}

/*!
	The nice and the right way to stop the playback.
*/
void CAPlayback::stop() {
	scoreViewPort()->setPlaying(false);
	_stop = true;
}

/*!
	Generates streams (elements lists) of playable elements (notes, rests) and stores it locally.
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
				midiDevice()->send(message);
				message.clear();
				
				message << (176 + staff->voiceAt(j)->midiChannel()); // set volume
				message << (7);
				message << (100);
				midiDevice()->send(message);
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
	for (int j=streamIdx(i); j<streamAt(i).size() && streamAt(i).at(j)->timeStart() <= curTime(i); streamIdx(i) = j++) {
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
