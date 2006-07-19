/** @file playback.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QPen>
#include <QRect>

#include <vector>	//needed for RtMidi send message
#include <iostream>

#include "playback.h"
#include "mididevice.h"
#include "scoreviewport.h"
#include "sheet.h"
#include "staff.h"
#include "context.h"
#include "drawablecontext.h"
#include "drawablemuselement.h"
#include "voice.h"

CAPlayback::CAPlayback(CAScoreViewPort *v, CAMidiDevice *m) {
	_scoreViewPort = v;
	_midiDevice = m;
	_stop = false;
}

void CAPlayback::run() {
	QPen p;
	p.setColor(Qt::green);
	p.setWidth(3);
	
	_scoreViewPort->setBorder(p);
	_scoreViewPort->setPlaying(true);	//set the deadlock for borders
	
	CASheet *sheet = _scoreViewPort->sheet();
	
	//list of all the music element lists (ie. streams) taken from all the contexts
	QList<QList<CAMusElement*>*> stream; 
	std::vector<unsigned char> message;	//midi 3-byte message sent to midi device

	QList<CADrawableMusElement *> oldSelection;
	oldSelection = *_scoreViewPort->selection();
	_scoreViewPort->clearSelection();

	for (int i=0; i < sheet->contextList()->size(); i++) {
		if (sheet->contextList()->at(i)->contextType() == CAContext::Staff) {
			CAStaff *staff = ((CAStaff*)(sheet->contextList()->at(i)));
			//add all the voices lists to the common list
			for (int j=0; j < staff->voiceCount(); j++) {
				stream << staff->voiceAt(j)->musElementList();
				message.push_back(192 + staff->voiceAt(j)->midiChannel());
				message.push_back(staff->voiceAt(j)->midiProgram());
				_midiDevice->send(&message);	//change program
				message.clear();				
				message.push_back(176 + staff->voiceAt(j)->midiChannel());
				message.push_back(7);
				message.push_back(100);
				_midiDevice->send(&message);	//set volume
				message.clear();				
			}
			
		}
	}
	
	int streams = stream.size();
	if (!streams) quit();
	int streamsIdx[streams]; for (int i=0; i<streams; i++) streamsIdx[i] = 0;
	CAClef *lastClef[streams]; for (int i=0; i<streams; i++) lastClef[i] = 0;
	
	QList<CAPlayable *> curPlaying;	//list of currently playing notes
	
	bool done;
	int timeStart;
	while (true) {
		timeStart = -1;
		for (int i=0; i<streams; i++) {
			while ( (streamsIdx[i] < stream[i]->size()) &&
			        ((stream[i]->at(streamsIdx[i])->musElementType() != CAMusElement::Note) &&
			        (stream[i]->at(streamsIdx[i])->musElementType() != CAMusElement::Rest))
			      )
				streamsIdx[i]++;
			
			if (streamsIdx[i] == stream[i]->size())
				continue;
			
			if ( (timeStart > stream[i]->at(streamsIdx[i])->timeStart()) || (timeStart == -1) )
				timeStart = stream[i]->at(streamsIdx[i])->timeStart();
		}
		
		CAPlayable *elt;
		CADrawableMusElement *drawable;
		//note off
		for (int i=0; i<curPlaying.size(); i++) {
			if (((elt = (CAPlayable*)curPlaying[i])->timeStart() + elt->timeLength()) == timeStart) {
				message.push_back(128 + elt->voice()->midiChannel());
				message.push_back(elt->midiPitch());
				message.push_back(127);
				_midiDevice->send(&message);	//release note
				message.clear();
				curPlaying.removeAt(i);
				
				_scoreViewPort->removeFromSelection(drawable = _scoreViewPort->find((CAMusElement*)elt));
				//_scoreViewPort->setRepaintArea(new QRect(drawable->xPos(), drawable->yPos(), drawable->width(), drawable->height()));			      	
			    //_scoreViewPort->repaint();
			      	
				i--;
			}
		}
		
		done = true;
		for (int i=0; i<streams; i++)
			if (stream[i]->size() != streamsIdx[i]) {
				done = false;
				break;
			}
		
		if (done || _stop) break;

		//note on
		for (int i=0; i<streams; i++) {
			while ( (stream[i]->size() > streamsIdx[i]) &&
			        ((elt = (CAPlayable*)stream[i]->at(streamsIdx[i]))->timeStart() == timeStart)
			      ) {
					message.push_back(144 + elt->voice()->midiChannel());
					message.push_back(elt->midiPitch());
					message.push_back(127);
					_midiDevice->send(&message);	//play note
					message.clear();
			      	curPlaying << elt;
					
					_scoreViewPort->addToSelection(drawable = _scoreViewPort->find((CAMusElement*)elt));
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
		
		msleep(minLength);
	}
	
	_scoreViewPort->clearSelection();
	_scoreViewPort->addToSelection(&oldSelection);
	_scoreViewPort->unsetBorder();
	//_scoreViewPort->repaint();
	stop();
}

void CAPlayback::stop() {
	_scoreViewPort->setPlaying(false);
	_stop = true;
}
