/** @file interface/engraver.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QList>
#include <QMap>
#include <iostream>	//debug
#include "interface/engraver.h"

#include "widgets/scoreviewport.h"

#include "drawable/drawablestaff.h"
#include "drawable/drawableclef.h"
#include "drawable/drawablenote.h"
#include "drawable/drawablerest.h"
#include "drawable/drawablekeysignature.h"
#include "drawable/drawabletimesignature.h"
#include "drawable/drawablebarline.h"
#include "drawable/drawableaccidental.h"

#include "drawable/drawablefunctionmarking.h"
#include "drawable/drawablefunctionmarkingcontext.h"

#include "core/sheet.h"

#include "core/staff.h"
#include "core/voice.h"
#include "core/keysignature.h"
#include "core/timesignature.h"

#include "core/functionmarkingcontext.h"
#include "core/functionmarking.h"

#define INITIAL_X_OFFSET 20
#define MINIMUM_SPACE 10

void CAEngraver::reposit(CAScoreViewPort *v) {
	int i;
	CASheet *sheet = v->sheet();
	
	//list of all the music element lists (ie. streams) taken from all the contexts
	QList<QList<CAMusElement*>*> musStreamList; 

	int dy = 50;
	QList<int> nonFirstVoiceIdxs;	//list of indexes of musStreamLists which the voices aren't the first voice. This is used later for determining should a sign be created or not (if it has been created in 1st voice already, don't recreate it in the other voices in the same staff).
	QMap<CAContext*, CADrawableContext*> drawableContextMap;
	
	for (int i=0; i < sheet->contextCount(); i++, dy+=100) {
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
		} else
		if (sheet->contextAt(i)->contextType() == CAContext::FunctionMarkingContext) {
			CAFunctionMarkingContext *fmContext = ((CAFunctionMarkingContext*)(sheet->contextAt(i)));
			drawableContextMap[fmContext] = new CADrawableFunctionMarkingContext(fmContext, 0, dy);
			v->addCElement(drawableContextMap[fmContext]);
			musStreamList << (QList<CAMusElement*>*)fmContext->functionMarkingList();
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
		for (int i=0; i<streams; i++)
			if (musStreamList[i]->last()->musElementType()!=CAMusElement::FunctionMarking)
				streamsX[i] = maxX;
		
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
			        (elt->musElementType() != CAMusElement::Barline) &&	//barlines should be aligned
			        (elt->musElementType() != CAMusElement::FunctionMarking)	//function markings are placed separately
			      ) {
				drawableContext = drawableContextMap[elt->context()];
				
				//place signs in first voices
				if ( (drawableContext->drawableContextType() == CADrawableContext::DrawableStaff) &&
				     (!nonFirstVoiceIdxs.contains(i)) ) {
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
					} /*SWITCH*/
				} /*IF firstVoice*/
				streamsIdx[i]++;
			}
		}
		
		//Draw function key name, if needed
		QList<CADrawableFunctionMarkingSupport*> lastDFMKeyNames;
		for (int i=0;
		     (i<streams) &&
		     (streamsIdx[i] < musStreamList[i]->size()) &&
			 ((elt = musStreamList[i]->at(streamsIdx[i]))->timeStart() == timeStart);
			 i++) {
			CAMusElement *elt = musStreamList[i]->at(streamsIdx[i]);
			if (elt->musElementType()==CAMusElement::FunctionMarking) {
				drawableContext = drawableContextMap[elt->context()];
				if (streamsIdx[i]-1<0 ||
				    ((CAFunctionMarking*)musStreamList[i]->at(streamsIdx[i]-1))->key() != ((CAFunctionMarking*)elt)->key()
				   ) {
					//draw new function marking key, if it was changed or if it's the first function in the score
					CADrawableFunctionMarkingSupport *support = new CADrawableFunctionMarkingSupport(
						CADrawableFunctionMarkingSupport::Key,
						((CAFunctionMarking*)elt)->key(),
						drawableContext,
						streamsX[i],
						((CADrawableFunctionMarkingContext*)drawableContext)->yPosLine(CADrawableFunctionMarkingContext::Middle)
					);
					streamsX[i] += (support->neededWidth());
					v->addMElement(support);
					lastDFMKeyNames << support;
				}
			}
		}
		
		//Synchronize minimum X-es between the contexts - all the noteheads or barlines should be horizontally aligned.
		for (int i=0; i<streams; i++) maxX = (streamsX[i] > maxX) ? streamsX[i] : maxX;
		for (int i=0; i<streams; i++)
			if (musStreamList[i]->last()->musElementType()!=CAMusElement::FunctionMarking)
				streamsX[i] = maxX;
		
		//Place barlines
		for (int i=0; i<streams; i++) {
			if (!(musStreamList[i]->size() > streamsIdx[i]) ||	//if the stream is already at the end, continue to the next stream
			    ((elt = musStreamList[i]->at(streamsIdx[i]))->timeStart() != timeStart))
				continue;
			
			if ((elt = musStreamList[i]->at(streamsIdx[i]))->musElementType() == CAMusElement::Barline) {
				if (nonFirstVoiceIdxs.contains(i))	{ //if the current stream is non-first voice, continue, as the barlines are drawn from the first voice only
					streamsIdx[i] = streamsIdx[i] + 1;
					continue;
				}
				
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
		
		//Place accidentals and key names of the function markings, if needed.
		//These elements are so called Support elements. They can't be selected and they're not really connected usually to any logical element, but they're needed when drawing.
		int maxWidth = 0;
		int maxAccidentalXEnd = 0;
		//CADrawableAccidental* noteAccs[streams];
		QList<CADrawableAccidental*> lastAccidentals; 
		for (int i=0; i < streams; i++) {
			//loop until the element has come, which has bigger timeStart
			CADrawableMusElement *newElt=0;
			int oldStreamIdx = streamsIdx[i];
			while ( (streamsIdx[i] < musStreamList[i]->size()) &&
			        ((elt = musStreamList[i]->at(streamsIdx[i]))->timeStart() == timeStart) &&
			        (elt->isPlayable())
			      ) {
				drawableContext = drawableContextMap[elt->context()];
				
				if (elt->musElementType()==CAMusElement::Note &&
				    ((CADrawableStaff*)drawableContext)->getAccs(streamsX[i], ((CANote*)elt)->pitch()) != ((CANote*)elt)->accidentals()
				   ) {
						newElt = new CADrawableAccidental(
							((CANote*)elt)->accidentals(),
							0,	//do not set the logical part as it gets selected by score viewport otherwise
							((CADrawableStaff*)drawableContext),
							streamsX[i],
							((CADrawableStaff*)drawableContext)->calculateCenterYCoord((CANote*)elt, lastClef[i])
						);

						v->addMElement(newElt);
						lastAccidentals << (CADrawableAccidental*)newElt;
						if (newElt->neededWidth() > maxWidth)
							maxWidth = newElt->neededWidth();
						if (maxAccidentalXEnd < newElt->neededWidth()+newElt->xPos())
							maxAccidentalXEnd = newElt->neededWidth()+newElt->xPos();
				}
				
				streamsIdx[i]++;
			}
			streamsIdx[i] = oldStreamIdx;
			
			streamsX[i] += (maxWidth?maxWidth+1:0);	//append the needed space for the last used note
		}
		
		//Synchronize minimum X-es between the contexts - all the noteheads or barlines should be horizontally aligned.
		for (int i=0; i<streams; i++) maxX = (streamsX[i] > maxX) ? streamsX[i] : maxX;
		for (int i=0; i<streams; i++) streamsX[i] = maxX;
		
		//Align support elements (accidentals, function key names) to the right
		for (int i=0; i<lastDFMKeyNames.size(); i++)
			lastDFMKeyNames[i]->setXPos(maxX - lastDFMKeyNames[i]->neededWidth());
		
		int deltaXPos = maxX - maxAccidentalXEnd;
		for (int i=0; i<lastAccidentals.size(); i++) {
			lastAccidentals[i]->setXPos(lastAccidentals[i]->xPos()+deltaXPos-1);
		}
		
		//place noteheads
		for (int i=0; i < streams; i++) {
			//loop until the element has come, which has bigger timeStart
			CADrawableMusElement *newElt=0;
			while ( (streamsIdx[i] < musStreamList[i]->size()) &&
			        ((elt = musStreamList[i]->at(streamsIdx[i]))->timeStart() == timeStart) &&
			        ((elt->isPlayable() || (elt->musElementType()==CAMusElement::FunctionMarking)))
			      ) {
				drawableContext = drawableContextMap[elt->context()];
				
				switch ( elt->musElementType() ) {
					case CAMusElement::Note: {
						newElt = new CADrawableNote(
							(CANote*)elt,
							drawableContext,
							streamsX[i],
							((CADrawableStaff*)drawableContext)->calculateCenterYCoord((CANote*)elt, lastClef[i])
						);

						v->addMElement(newElt);
						streamsX[i] += (newElt->neededWidth() + MINIMUM_SPACE);
						break;
					}
					case CAMusElement::Rest: {
						newElt = new CADrawableRest(
							(CARest*)elt,
							drawableContext,
							streamsX[i],
							drawableContext->yPos()
						);

						v->addMElement(newElt);
						streamsX[i] += (newElt->neededWidth() + MINIMUM_SPACE);
						break;
					}
					case CAMusElement::FunctionMarking: {
						CAFunctionMarking *function = (CAFunctionMarking*)elt;
						
						newElt = new CADrawableFunctionMarking(
							function,
							(CADrawableFunctionMarkingContext*)drawableContext,
							streamsX[i],
							function->tonicDegree()?
								((CADrawableFunctionMarkingContext*)drawableContext)->yPosLine(CADrawableFunctionMarkingContext::Upper):
								((CADrawableFunctionMarkingContext*)drawableContext)->yPosLine(CADrawableFunctionMarkingContext::Middle)
						);
						
						//tonicization
						int j=streamsIdx[i]-1;
						CADrawableFunctionMarkingSupport *tonicization=0;
						if (j>=0 && ((CAFunctionMarking*)musStreamList[i]->at(j))->tonicDegree()!=CAFunctionMarking::None
							&& (function->tonicDegree()!=((CAFunctionMarking*)musStreamList[i]->at(j))->tonicDegree()) ) {
							CAFunctionMarking::CAFunctionType type = ((CAFunctionMarking*)musStreamList[i]->at(j))->tonicDegree();
							while (--j>=0 && ((CAFunctionMarking*)musStreamList[i]->at(j))->tonicDegree()==((CAFunctionMarking*)musStreamList[i]->at(j+1))->tonicDegree());
							CAFunctionMarking *tonicStart = (CAFunctionMarking*)musStreamList[i]->at(++j);
							CADrawableFunctionMarking *left, *right=0;
							left = (CADrawableFunctionMarking*)drawableContext->findMElement(tonicStart);
							if (tonicStart!=(CAMusElement*)musStreamList[i]->at(streamsIdx[i]-1)) {
								right = (CADrawableFunctionMarking*)drawableContext->findMElement((CAMusElement*)musStreamList[i]->at(streamsIdx[i]-1));
								tonicization = new CADrawableFunctionMarkingSupport(
									CADrawableFunctionMarkingSupport::Tonicization,
									left,
									(CADrawableFunctionMarkingContext*)drawableContext,
									left->xPos(),
									((CADrawableFunctionMarkingContext*)drawableContext)->yPosLine(CADrawableFunctionMarkingContext::Middle),
									right
								);
							} else {
								tonicization = new CADrawableFunctionMarkingSupport(
									CADrawableFunctionMarkingSupport::Tonicization,
									left,
									(CADrawableFunctionMarkingContext*)drawableContext,
									left->xPos(),
									((CADrawableFunctionMarkingContext*)drawableContext)->yPosLine(CADrawableFunctionMarkingContext::Middle)
								);
								tonicization->setXPos((int)(left->xPos()+0.5*left->width()-0.5*tonicization->width()+0.5));	//align center
							}
						}
						
						//set extender line and change the width of the previous function marking if needed
						if (drawableContext->lastDrawableMusElement()!=0 && drawableContext->lastDrawableMusElement()->drawableMusElementType()==CADrawableMusElement::DrawableFunctionMarking) {
							CAFunctionMarking *prevElt = (CAFunctionMarking*)drawableContext->lastDrawableMusElement()->musElement();
							QList<CANote*> chord = prevElt->context()->sheet()->getChord(prevElt->timeStart());
							for (int i=0; i<chord.size(); i++) {
								if (chord[i]->timeLength()<prevElt->timeLength()) {
									((CADrawableFunctionMarking*)drawableContext->lastDrawableMusElement())->setExtenderLineVisible(true);
									drawableContext->lastDrawableMusElement()->setWidth(newElt->xPos()-drawableContext->lastDrawableMusElement()->xPos()-5);
									break;
								}
							}
						}
						
						v->addMElement(newElt);
						if (tonicization) v->addMElement(tonicization);
						
						//draw chord area, if needed
						if (function->chordArea()!=CAFunctionMarking::None) {
							CADrawableFunctionMarkingSupport *chordArea = new CADrawableFunctionMarkingSupport(
								CADrawableFunctionMarkingSupport::ChordArea,
								(CADrawableFunctionMarking*)newElt,
								(CADrawableFunctionMarkingContext*)drawableContext,
								streamsX[i],
								((CADrawableFunctionMarkingContext*)drawableContext)->yPosLine(CADrawableFunctionMarkingContext::Lower)								
							);
							chordArea->setXPos((int)(newElt->xPos()-(chordArea->width()-newElt->width())/2.0 + 0.5));
							v->addMElement(chordArea);
						}
						
						streamsX[i] += (newElt->neededWidth());
						break;
					}
				}
				
				streamsIdx[i]++;
			}
		}
	}
}
