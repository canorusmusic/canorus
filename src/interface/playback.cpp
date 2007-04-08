/** @file playback.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QPen>
#include <QRect>
#include <QVector>	//needed for RtMidi send message

#include <iostream>

#include "interface/playback.h"
#include "interface/mididevice.h"
#include "widgets/scoreviewport.h"
#include "core/sheet.h"
#include "core/staff.h"
#include "core/context.h"
#include "drawable/drawablecontext.h"
#include "drawable/drawablemuselement.h"
#include "core/voice.h"

CAPlayback::CAPlayback(CAScoreViewPort *v, CAMidiDevice *m) {
	_scoreViewPort = v;
	_midiDevice = m;
	_stop = false;
	_currentTime = 0;
}

void CAPlayback::run() {
	QPen p;
	p.setColor(Qt::green);
	p.setWidth(3);
	
	_scoreViewPort->setBorder(p);
	_scoreViewPort->setPlaying(true);	// set the deadlock for borders
	
	CASheet *sheet = _scoreViewPort->sheet();
	
	// list of all the music element lists (ie. streams) taken from all the contexts
	QList< QList<CAMusElement*> > stream; 
	QVector<unsigned char> message;	// midi 3-byte message sent to midi device

	QList<CADrawableMusElement *> oldSelection;
	oldSelection = _scoreViewPort->selection();
	_scoreViewPort->clearSelection();

	for (int i=0; i < sheet->contextCount(); i++) {
		if (sheet->contextAt(i)->contextType() == CAContext::Staff) {
			CAStaff *staff = ((CAStaff*)(sheet->contextAt(i)));
			// add all the voices lists to the common list stream
			for (int j=0; j < staff->voiceCount(); j++) {
				stream << staff->voiceAt(j)->musElementList();
				
				message << (192 + staff->voiceAt(j)->midiChannel()); // change program
				message << (staff->voiceAt(j)->midiProgram());
				_midiDevice->send(message);
				message.clear();
				
				message << (176 + staff->voiceAt(j)->midiChannel()); // set volume
				message << (7);
				message << (100);
				_midiDevice->send(message);
				message.clear();
			}
			
		}
	}
	
	int streams = stream.size();
	if (!streams) quit();
	int streamsIdx[streams]; for (int i=0; i<streams; i++) streamsIdx[i] = 0;
	CAClef *lastClef[streams]; for (int i=0; i<streams; i++) lastClef[i] = 0;
	
	QList<CANote *> curPlaying;	//list of currently playing notes
	
	bool done;
	int timeStart;
	while (true) {
		timeStart = -1;
		for (int i=0; i<streams; i++) {
			while ( (streamsIdx[i] < stream[i].size()) &&
			        ((stream[i].at(streamsIdx[i])->musElementType() != CAMusElement::Note) &&
			        (stream[i].at(streamsIdx[i])->musElementType() != CAMusElement::Rest))
			      )
				streamsIdx[i]++;
			
			if (streamsIdx[i] == stream[i].size())
				continue;
			
			if ( (timeStart > stream[i].at(streamsIdx[i])->timeStart()) || (timeStart == -1) )
				timeStart = stream[i].at(streamsIdx[i])->timeStart();
		}
		
		CANote *note;
		CADrawableMusElement *drawable;
		// note off
		for (int i=0; i<curPlaying.size(); i++) {
			if ( ((note = curPlaying[i])->timeStart() + note->timeLength() == timeStart) ) {
				message << (128 + note->voice()->midiChannel()); // note off
				message << (note->midiPitch());
				message << (127);
				if (! (note->tieStart() && note->tieStart()->noteEnd()) )
					_midiDevice->send(message);
				message.clear();
				curPlaying.removeAt(i);
				
				_scoreViewPort->removeFromSelection(drawable = _scoreViewPort->findMElement(note));
				
				//_scoreViewPort->setRepaintArea(new QRect(drawable->xPos(), drawable->yPos(), drawable->width(), drawable->height()));			      	
			    //_scoreViewPort->repaint();
			      	
				i--;
			}
		}
		
		done = true;
		for (int i=0; i<streams; i++)
			if (stream[i].size() != streamsIdx[i]) {
				done = false;
				break;
			}
		
		if (done || _stop) break;

		// note on
		for (int i=0; i<streams; i++) {
			while ( (stream[i].size() > streamsIdx[i]) &&
			        stream[i].at(streamsIdx[i])->musElementType()==CAMusElement::Note &&
			        ((note = static_cast<CANote*>(stream[i].at(streamsIdx[i])))->timeStart() == timeStart)
			      ) {
				message << (144 + note->voice()->midiChannel()); // note on
				message << (note->midiPitch());
				message << (127);
				if ( note->musElementType()!=CAMusElement::Rest && !note->tieEnd() )
					_midiDevice->send(message);
				message.clear();
				curPlaying << note;
				
				_scoreViewPort->addToSelection( drawable = _scoreViewPort->findMElement( note ) );
				
				//_scoreViewPort->setRepaintArea(new QRect(drawable->xPos(), drawable->yPos(), drawable->width(), drawable->height()));			      	
				//_scoreViewPort->repaint();
				
				streamsIdx[i]++;
			}
		}
		
		int minLength = -1;
		int delta;
		for (int i=0; i<curPlaying.size(); i++) {
			if (((delta = (curPlaying[i]->timeStart() + curPlaying[i]->midiLength() - timeStart)) < minLength) || (minLength == -1))
				minLength = delta;
		}
		
		_currentTime = timeStart;
		msleep(minLength);
	}
	
	_scoreViewPort->clearSelection();
	_scoreViewPort->addToSelection( oldSelection );
	_scoreViewPort->unsetBorder();
	stop();
}

void CAPlayback::stop() {
	_scoreViewPort->setPlaying(false);
	_stop = true;
}
