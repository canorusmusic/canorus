/** @file engraver.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QList>
#include <QMap>
#include <iostream>
#include "interface/engraver.h"

#include "widgets/scoreviewport.h"
#include "drawable/drawablestaff.h"
#include "drawable/drawableclef.h"
#include "drawable/drawablenote.h"
#include "drawable/drawablekeysignature.h"
#include "drawable/drawabletimesignature.h"
#include "drawable/drawablebarline.h"

#include "core/sheet.h"
#include "core/staff.h"
#include "core/voice.h"
#include "core/keysignature.h"
#include "core/timesignature.h"

#define INITIAL_X_OFFSET 20
#define MINIMUM_SPACE 10

void CAEngraver::reposit(CAScoreViewPort *v) {
	int i;
	CASheet *sheet = v->sheet();
	
	//list of all the music element lists (ie. streams) taken from all the contexts
	QList<QList<CAMusElement*>*> musStreamList; 

	int dy = 50;
	QList<int> nonFirstVoiceIdxs;	//list of indexes of musStreamLists which the voices aren't the first voice. This is used later for determining should a sign be created or not (if it has been created in 1st voice already, don't recreate it in the other voices in th same staff).
	QMap<CAContext*, CADrawableContext*> drawableContextMap;
	
	for (int i=0; i < sheet->contextCount(); i++, dy+=200) {
		if (sheet->contextAt(i)->contextType() == CAContext::Staff) {
			CAStaff *staff = ((CAStaff*)(sheet->contextAt(i)));
			drawableContextMap[staff] = new CADrawableStaff(staff, 0, dy);
			v->addCElement(drawableContextMap[staff]);
			
			//add all the voices lists to the common list
			for (int j=0; j < staff->voiceCount(); j++) {
				musStreamList << staff->voiceAt(j)->musElementList();
				if (staff->voiceAt(j)->voiceNumber()!=1)
					nonFirstVoiceIdxs << j;
			}
			
		}
	}
	
	int streams = musStreamList.size();
	int streamsIdx[streams]; for (int i=0; i<streams; i++) streamsIdx[i] = 0;
	int streamsX[streams]; for (int i=0; i<streams; i++) streamsX[i] = INITIAL_X_OFFSET;
	CAClef *lastClef[streams]; for (int i=0; i<streams; i++) lastClef[i] = 0;
	CAKeySignature *lastKeySig[streams]; for (int i=0; i<streams; i++) lastKeySig[i] = 0;
	CATimeSignature *lastTimeSig[streams]; for (int i=0; i<streams; i++) lastTimeSig[i] = 0;

	int timeStart = 0;
	bool done = false;
	while (!done) {
		//Synchronize minimum X-es between the contexts
		int maxX = 0;
		for (int i=0; i<streams; i++) maxX = (streamsX[i] > maxX) ? streamsX[i] : maxX;
		for (int i=0; i<streams; i++) streamsX[i] = maxX;
		
		//if all the indices are at the end of the streams, finish.
		int idx;
		for (idx=0; (idx < streams) && (streamsIdx[idx] == musStreamList[idx]->size()); idx++);
		if (idx==streams) { done=true; continue; }

		//go through all the streams and remember the time of the soonest element that will happen
		timeStart = -1;	//reset the timeStart - the next one minimum will be set in the following loop
		for (int idx=0; idx < streams; idx++) {
			if ( (musStreamList[idx]->size() > streamsIdx[idx]) && ((timeStart==-1) || (musStreamList[idx]->at(streamsIdx[idx])->timeStart() < timeStart)) )
				timeStart = musStreamList[idx]->at(streamsIdx[idx])->timeStart();
		}
		//timeStart now holds the nearest next time we're going to draw
		
		
		//go through all the streams and check if the following element has this time
		CAMusElement *elt;
		CADrawableContext *drawableContext;
		bool placedSymbol = false;	//used if waiting for notes to gather and a non-time-consuming symbol has been placed
		for (int i=0; i < streams; i++) {
			//loop until the first playable element
			//multiple elements can have the same start time. eg. Clef + Key signature + Time signature + First note
			while ( (streamsIdx[i] < musStreamList[i]->size()) &&
			        ((elt = musStreamList[i]->at(streamsIdx[i]))->timeStart() == timeStart) &&
			        (!elt->isPlayable()) &&
			        (elt->musElementType() != CAMusElement::Barline)	//barlines should be aligned
			      ) {
				drawableContext = drawableContextMap[elt->context()];
				
				//place signs in first voices
				if ( (drawableContext->drawableContextType() == CADrawableContext::DrawableStaff) &&
				     (!nonFirstVoiceIdxs.contains(i)) )
				 {
					switch ( elt->musElementType() ) {
						case CAMusElement::Clef: {
							CADrawableClef *clef = new CADrawableClef(
								(CAClef*)elt,
								(CADrawableStaff*)drawableContext,
								streamsX[i],
								drawableContext->yPos()
							);
							
							v->addMElement(clef);
							lastClef[i] = clef->clef();
							
							streamsX[i] += (clef->neededWidth() + MINIMUM_SPACE);
							placedSymbol = true;
							break;
						}
						case CAMusElement::KeySignature: {
							CADrawableKeySignature *keySig = new CADrawableKeySignature(
								(CAKeySignature*)elt,
								(CADrawableStaff*)drawableContext,
								streamsX[i],
								drawableContext->yPos()
							);
							
							v->addMElement(keySig);
							lastKeySig[i] = keySig->keySignature();
							
							streamsX[i] += (keySig->neededWidth() + MINIMUM_SPACE);
							placedSymbol = true;
							break;
						}
						case CAMusElement::TimeSignature: {
							CADrawableTimeSignature *timeSig = new CADrawableTimeSignature(
								(CATimeSignature*)elt,
								(CADrawableStaff*)drawableContext,
								streamsX[i],
								drawableContext->yPos()
							);
							
							v->addMElement(timeSig);
							lastTimeSig[i] = timeSig->timeSignature();
							
							streamsX[i] += (timeSig->neededWidth() + MINIMUM_SPACE);
							placedSymbol = true;
							break;
						}
					}
				}
				
				streamsIdx[i] = streamsIdx[i] + 1;
			}
		}
		
		//Synchronize minimum X-es between the contexts - all the noteheads or barlines should be horizontally aligned.
		if (placedSymbol) {
			for (int i=0; i<streams; i++) maxX = (streamsX[i] > maxX) ? streamsX[i] : maxX;
			for (int i=0; i<streams; i++) streamsX[i] = maxX;
		}
		
		//place barlines
		for (int i=0; i<streams; i++) {
			if (!(musStreamList[i]->size() > streamsIdx[i]))
				continue;
			
			if ((elt = musStreamList[i]->at(streamsIdx[i]))->musElementType() == CAMusElement::Barline) {
				drawableContext = drawableContextMap[elt->context()];
				CADrawableBarline *bar = new CADrawableBarline(
					(CABarline*)elt,
					(CADrawableStaff*)drawableContext,
					streamsX[i],
					drawableContext->yPos()
				);
				
				v->addMElement(bar);
				placedSymbol = true;
				streamsX[i] += (bar->neededWidth() + MINIMUM_SPACE);
				streamsIdx[i] = streamsIdx[i] + 1;
			}
		}
		
		if (placedSymbol)	//always start adding notes cleanly 
			continue;
			
		for (int i=0; i < streams; i++) {
			//loop until the element has come, which has bigger timeStart
			CADrawableNote *note = 0;
			while ( (streamsIdx[i] < musStreamList[i]->size()) &&
			        ((elt = musStreamList[i]->at(streamsIdx[i]))->timeStart() == timeStart) &&
			        (elt->isPlayable())
			      ) {
				drawableContext = drawableContextMap[elt->context()];
				bool b = elt->isPlayable();
				int type = elt->musElementType();
				
				switch ( elt->musElementType() ) {
					case CAMusElement::Note:
						note = new CADrawableNote(
							(CANote*)elt,
							drawableContext,
							streamsX[i],
							((CADrawableStaff*)drawableContext)->calculateCenterYCoord((CANote*)elt, lastClef[i])
						);

						v->addMElement(note);

						break;
				}
				
				streamsIdx[i] = streamsIdx[i] + 1;
			}

			if (note)
				streamsX[i] += (note->neededWidth() + MINIMUM_SPACE);	//append the needed space for the last used note
		}
	}
}
