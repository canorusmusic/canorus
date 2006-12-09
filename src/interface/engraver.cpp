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
	CADrawableFunctionMarkingSupport *lastDFMTonicizations[streams]; for (int i=0; i<streams; i++) lastDFMTonicizations[i]=0;
	while (!done) {	
		//if all the indices are at the end of the streams, finish.
		int idx;
		for (idx=0; (idx < streams) && (streamsIdx[idx] == musStreamList[idx]->size()); idx++);
		if (idx==streams) { done=true; continue; }
		
		//Synchronize minimum X-es between the contexts
		int maxX = 0;		
		for (int i=0; i<streams; i++) maxX = (streamsX[i] > maxX) ? streamsX[i] : maxX;
		for (int i=0; i<streams; i++)
			if (musStreamList[i]->last()->musElementType()!=CAMusElement::FunctionMarking)
				streamsX[i] = maxX;
		
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
			if (elt->musElementType()==CAMusElement::FunctionMarking && ((CAFunctionMarking*)elt)->function()!=CAFunctionMarking::None) {
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
			lastDFMKeyNames[i]->setXPos(maxX - lastDFMKeyNames[i]->neededWidth() - 2);
		
		int deltaXPos = maxX - maxAccidentalXEnd;
		for (int i=0; i<lastAccidentals.size(); i++) {
			lastAccidentals[i]->setXPos(lastAccidentals[i]->xPos()+deltaXPos-1);
		}
		
		//Place noteheads and other elements aligned to noteheads
		for (int i=0; i < streams; i++) {
			//loop until the element has come, which has bigger timeStart
			while ( (streamsIdx[i] < musStreamList[i]->size()) &&
			        ((elt = musStreamList[i]->at(streamsIdx[i]))->timeStart() == timeStart) &&
			        ((elt->isPlayable() || (elt->musElementType()==CAMusElement::FunctionMarking)))
			      ) {
				drawableContext = drawableContextMap[elt->context()];
				CADrawableMusElement *newElt=0;
				
				switch ( elt->musElementType() ) {
					case CAMusElement::Note: {
						newElt = new CADrawableNote(
							(CANote*)elt,
							drawableContext,
							streamsX[i],
							((CADrawableStaff*)drawableContext)->calculateCenterYCoord((CANote*)elt, lastClef[i])
						);

						v->addMElement(newElt);
						if (((CANote*)elt)->isLastInTheChord())	
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
						
						//Make a new line, if parallel function present
						if (streamsIdx[i]-1>=0 && musStreamList[i]->at(streamsIdx[i]-1)->timeStart()==musStreamList[i]->at(streamsIdx[i])->timeStart()) {
							((CADrawableFunctionMarkingContext*)drawableContext)->nextLine();
							CADrawableFunctionMarkingSupport *newKey = new CADrawableFunctionMarkingSupport(
								CADrawableFunctionMarkingSupport::Key,
								function->key(),
								drawableContext,
								streamsX[i],
								((CADrawableFunctionMarkingContext*)drawableContext)->yPosLine(CADrawableFunctionMarkingContext::Middle)
							);
							newKey->setXPos(streamsX[i]-newKey->width()-2);
							v->addMElement(newKey);
						}
						
						//Place the function itself, if it's independent
						if (function->function()!=CAFunctionMarking::None)
							newElt = new CADrawableFunctionMarking(
								function,
								(CADrawableFunctionMarkingContext*)drawableContext,
								streamsX[i],
								function->tonicDegree()?
									((CADrawableFunctionMarkingContext*)drawableContext)->yPosLine(CADrawableFunctionMarkingContext::Upper):
									((CADrawableFunctionMarkingContext*)drawableContext)->yPosLine(CADrawableFunctionMarkingContext::Middle)
							);
						
						//Place alterations
						CADrawableFunctionMarkingSupport *alterations=0;
						if (function->addedDegrees().size() || function->alteredDegrees().size()) {
							alterations = new CADrawableFunctionMarkingSupport(
								CADrawableFunctionMarkingSupport::Alterations,
								function,
								drawableContext,
								streamsX[i],
								((CADrawableFunctionMarkingContext*)drawableContext)->yPosLine(CADrawableFunctionMarkingContext::Lower)+3
							);
							//center-align alterations to function, if placed
							if (newElt)
								alterations->setXPos((int)(newElt->xPos()+newElt->width()/2.0-alterations->width()/2.0+0.5));
							else	//center-align to note
								alterations->setXPos((int)(streamsX[i]+5-alterations->width()/2.0+0.5));								
						}
						
						//Place tonicization. The same tonicization is always placed from streamsIdx[i]-nth to streamsIdx[i]-1th element, where streamsIdx[i] is the current index. 
						int j=streamsIdx[i]-1;	//index of the previous elt
						CADrawableFunctionMarkingSupport *tonicization=0;
						for (; j>=0 && ((CAFunctionMarking*)musStreamList[i]->at(j))->function()==CAFunctionMarking::None; j--);	//ignore any alterations back there
						if (j>=0 && ((CAFunctionMarking*)musStreamList[i]->at(j))->tonicDegree()!=CAFunctionMarking::None	//place tonicization, if tonic degree is set...
							&& (function->tonicDegree()!=((CAFunctionMarking*)musStreamList[i]->at(j))->tonicDegree() || function->key()!=((CAFunctionMarking*)musStreamList[i]->at(j))->key()) ) {	//and it's not still the same
							CAFunctionMarking::CAFunctionType type = ((CAFunctionMarking*)musStreamList[i]->at(j))->tonicDegree();
							CAFunctionMarking *right = (CAFunctionMarking*)musStreamList[i]->at(j);
							
							//find the n-th element back
							while (--j>=0 && ((CAFunctionMarking*)musStreamList[i]->at(j))->tonicDegree()==((CAFunctionMarking*)musStreamList[i]->at(j+1))->tonicDegree() &&
							       ((CAFunctionMarking*)musStreamList[i]->at(j))->key()==((CAFunctionMarking*)musStreamList[i]->at(j+1))->key());
							CAFunctionMarking *tonicStart = (CAFunctionMarking*)musStreamList[i]->at(++j);
							CADrawableFunctionMarking *left = (CADrawableFunctionMarking*)drawableContext->findMElement(tonicStart);
							if (tonicStart!=(CAMusElement*)musStreamList[i]->at(streamsIdx[i]-1)) {	//tonicization isn't single (more than 1 tonic element)
								tonicization = new CADrawableFunctionMarkingSupport(
									CADrawableFunctionMarkingSupport::Tonicization,
									left,
									(CADrawableFunctionMarkingContext*)drawableContext,
									left->xPos(),
									((CADrawableFunctionMarkingContext*)drawableContext)->yPosLine(CADrawableFunctionMarkingContext::Middle),
									(CADrawableFunctionMarking*)drawableContext->findMElement(right)
								);
							} else {																//tonicization is single (one tonic)
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
						
						//Place horizontal modulation rectangle, if needed
						j=streamsIdx[i]-1;
						CADrawableFunctionMarkingSupport *hModulationRect=0;
						if (newElt && j>=0) {
							while (((CAFunctionMarking*)musStreamList[i]->at(j))->function()==CAFunctionMarking::None) j--;
							if (((CAFunctionMarking*)musStreamList[i]->at(j))->key()!=function->key() &&
							    ((CAFunctionMarking*)musStreamList[i]->at(j))->timeStart()!=function->timeStart()) {
								CADrawableFunctionMarking *left = (CADrawableFunctionMarking*)drawableContext->findMElement(musStreamList[i]->at(j));
								hModulationRect = new CADrawableFunctionMarkingSupport(
									CADrawableFunctionMarkingSupport::Rectangle,
									left,
									(CADrawableFunctionMarkingContext*)drawableContext,
									left->xPos(),
									left->yPos(),
									(CADrawableFunctionMarking*)newElt
								);
								if (streamsIdx[i]%2)
									hModulationRect->setRectWider(true);	//make it wider, so it potentially doesn't overlap with other rectangles around
						    }
						}
						
						//Place vertical modulation rectangle, if needed
						//This must be done *before* the extender lines are placed!
						j=streamsIdx[i]-1;
						CADrawableFunctionMarkingSupport *vModulationRect=0;
						while (--j>=0 &&
						       ((CAFunctionMarking*)musStreamList[i]->at(j))->key()!=((CAFunctionMarking*)musStreamList[i]->at(j+1))->key() &&
						       ((CAFunctionMarking*)musStreamList[i]->at(j))->timeStart()==((CAFunctionMarking*)musStreamList[i]->at(j+1))->timeStart()
						      );
						if (++j>=0 && j!=streamsIdx[i]-1) {
							CADrawableFunctionMarking *left = (CADrawableFunctionMarking*)drawableContext->findMElement(musStreamList[i]->at(j));
							vModulationRect = new CADrawableFunctionMarkingSupport(
								CADrawableFunctionMarkingSupport::Rectangle,
								left,
								(CADrawableFunctionMarkingContext*)drawableContext,
								left->xPos(),
								left->yPos(),
								(CADrawableFunctionMarking*)drawableContext->findMElement(musStreamList[i]->at(streamsIdx[i]-1))
							);
						}
						
						//Place horizontal chord area rectangle, if element neighbours are of the same chordarea/function
						j=streamsIdx[i]-1;
						CADrawableFunctionMarkingSupport *hChordAreaRect=0;
						if (j>=0 && //don't draw rectangle, if the current element would still be in the rectangle
						    (((CAFunctionMarking*)musStreamList[i]->at(j))->key()==function->key() && ((CAFunctionMarking*)musStreamList[i]->at(j))->function()!=function->function() && ((CAFunctionMarking*)musStreamList[i]->at(j))->function()!=function->chordArea() && ((CAFunctionMarking*)musStreamList[i]->at(j))->chordArea()!=function->chordArea() ||
						     ((CAFunctionMarking*)musStreamList[i]->at(j))->key()!=function->key() || j==musStreamList[i]->size()
						    )
						   ) {
							while (--j>=0 &&
							       ((CAFunctionMarking*)musStreamList[i]->at(j+1))->chordArea()!=CAFunctionMarking::None &&
							       ((CAFunctionMarking*)musStreamList[i]->at(j))->key()==((CAFunctionMarking*)musStreamList[i]->at(j+1))->key() &&
							        (((CAFunctionMarking*)musStreamList[i]->at(j))->chordArea()==((CAFunctionMarking*)musStreamList[i]->at(j+1))->chordArea() ||
							         ((CAFunctionMarking*)musStreamList[i]->at(j))->function()==((CAFunctionMarking*)musStreamList[i]->at(j+1))->chordArea() ||
							         ((CAFunctionMarking*)musStreamList[i]->at(j))->chordArea()==((CAFunctionMarking*)musStreamList[i]->at(j+1))->function()
							        )
							      );
							if (++j != streamsIdx[i]-1) {
								CADrawableFunctionMarking *left = (CADrawableFunctionMarking*)drawableContext->findMElement(musStreamList[i]->at(j));
								hChordAreaRect = new CADrawableFunctionMarkingSupport(
									CADrawableFunctionMarkingSupport::Rectangle,
									left,
									(CADrawableFunctionMarkingContext*)drawableContext,
									left->xPos(),
									left->yPos(),
									(CADrawableFunctionMarking*)drawableContext->findMElement(musStreamList[i]->at(streamsIdx[i]-1))
								);
							}
						}
						
						//Place chordarea marking below in paranthesis, if no neighbours are of same chordarea
						CADrawableFunctionMarkingSupport *chordArea=0;
						j=streamsIdx[i];
						if (newElt && function->chordArea()!=CAFunctionMarking::None &&
						    (j-1<0 ||
						     (((CAFunctionMarking*)musStreamList[i]->at(j-1))->key()==function->key() &&
						      ((CAFunctionMarking*)musStreamList[i]->at(j-1))->chordArea()!=function->chordArea() &&
						      ((CAFunctionMarking*)musStreamList[i]->at(j-1))->function()!=function->chordArea() &&
						      ((CAFunctionMarking*)musStreamList[i]->at(j-1))->chordArea()!=function->function() ||
						      ((CAFunctionMarking*)musStreamList[i]->at(j-1))->tonicDegree()!=function->tonicDegree() ||
						      ((CAFunctionMarking*)musStreamList[i]->at(j-1))->key()!=function->key()
						     )
						    ) &&
						    (j+1>=musStreamList[i]->size() ||
						     (((CAFunctionMarking*)musStreamList[i]->at(j+1))->key()==function->key() &&
						      ((CAFunctionMarking*)musStreamList[i]->at(j+1))->chordArea()!=function->chordArea() &&
						      ((CAFunctionMarking*)musStreamList[i]->at(j+1))->function()!=function->chordArea() &&
						      ((CAFunctionMarking*)musStreamList[i]->at(j+1))->chordArea()!=function->function() ||
						      ((CAFunctionMarking*)musStreamList[i]->at(j+1))->tonicDegree()!=function->tonicDegree() ||
						      ((CAFunctionMarking*)musStreamList[i]->at(j+1))->key()!=function->key()
						     )
						    )
						   ) {
							chordArea = new CADrawableFunctionMarkingSupport(
								CADrawableFunctionMarkingSupport::ChordArea,
								(CADrawableFunctionMarking*)newElt,
								(CADrawableFunctionMarkingContext*)drawableContext,
								streamsX[i],
								((CADrawableFunctionMarkingContext*)drawableContext)->yPosLine(CADrawableFunctionMarkingContext::Lower)								
							);
							chordArea->setXPos((int)(newElt->xPos()-(chordArea->width()-newElt->width())/2.0 + 0.5));
						}
						
						//Place ellipse
						j=streamsIdx[i]-1;
						CADrawableFunctionMarkingSupport *ellipse=0;
						if (j>=0 && ((CAFunctionMarking*)musStreamList[i]->at(j))->isPartOfEllipse()	//place ellipse, if it has it
							&& (!function->isPartOfEllipse()) ) {	//and it's not lasting anymore
							//find the n-th element back
							while (--j>=0 && ((CAFunctionMarking*)musStreamList[i]->at(j))->isPartOfEllipse());
							CAFunctionMarking *ellipseStart = (CAFunctionMarking*)musStreamList[i]->at(++j);
							CADrawableFunctionMarking *left = (CADrawableFunctionMarking*)drawableContext->findMElement(ellipseStart);
							ellipse = new CADrawableFunctionMarkingSupport(
								CADrawableFunctionMarkingSupport::Ellipse,
								left,
								(CADrawableFunctionMarkingContext*)drawableContext,
								left->xPos(),
								((CADrawableFunctionMarkingContext*)drawableContext)->yPosLine(CADrawableFunctionMarkingContext::Lower),
								(CADrawableFunctionMarking*)drawableContext->findMElement((CAMusElement*)musStreamList[i]->at(streamsIdx[i]-1))
							);
						}
						
						//set extender line and change the width of the previous function marking if needed
						if (newElt && streamsIdx[i]-1>=0 && musStreamList[i]->at(streamsIdx[i]-1)->timeStart()!=musStreamList[i]->at(streamsIdx[i])->timeStart()) {
							CAFunctionMarking *prevElt;
							for (int j=1; (prevElt=(CAFunctionMarking*)musStreamList[i]->at(streamsIdx[i]-j))->function()==CAFunctionMarking::None; j++);	//get the first real function (skip alterations)
							QList<CAPlayable*> chord = prevElt->context()->sheet()->getChord(prevElt->timeStart());
							for (int i=0; i<chord.size(); i++) {
								if (chord[i]->timeLength()<prevElt->timeLength()) {
									int newX;
									if (prevElt->tonicDegree()==CAFunctionMarking::None) {
										CADrawableFunctionMarking *prevDElt;
										prevDElt = (CADrawableFunctionMarking*)drawableContext->findMElement(prevElt);
										prevDElt->setExtenderLineVisible(true);
										if (prevElt->key()==((CADrawableFunctionMarking*)newElt)->functionMarking()->key())
											newX = newElt->xPos()-prevDElt->xPos();
										else
											newX = lastDFMKeyNames[i]->xPos()-prevDElt->xPos();										
										prevDElt->setWidth(newX);
									} else {
										if (tonicization) {
											tonicization->setExtenderLineVisible(true);
											if (prevElt->key()==((CADrawableFunctionMarking*)newElt)->functionMarking()->key())
												newX = newElt->xPos()-tonicization->xPos();
											else
												newX = lastDFMKeyNames[i]->xPos()-tonicization->xPos();
											tonicization->setWidth(newX);
										} else if (lastDFMTonicizations[i]) {
											lastDFMTonicizations[i]->setExtenderLineVisible(true);
											if (prevElt->key()==((CADrawableFunctionMarking*)newElt)->functionMarking()->key())
												newX = newElt->xPos()-lastDFMTonicizations[i]->xPos();
											else
												newX = lastDFMKeyNames[i]->xPos()-lastDFMTonicizations[i]->xPos();
											lastDFMTonicizations[i]->setWidth(newX);
										}
									}
									break;
								}
							}
						}
						
						if (newElt) v->addMElement(newElt);	//when only alterations are made and no function placed, IF is needed
						if (tonicization) { v->addMElement(tonicization); lastDFMTonicizations[i]=tonicization; }
						if (ellipse) v->addMElement(ellipse);
						if (hModulationRect) v->addMElement(hModulationRect);
						if (vModulationRect) v->addMElement(vModulationRect);
						if (hChordAreaRect) v->addMElement(hChordAreaRect);
						if (chordArea) v->addMElement(chordArea);
						if (alterations) v->addMElement(alterations);
						
						if (newElt && streamsIdx[i]+1<musStreamList[i]->size() && musStreamList[i]->at(streamsIdx[i]+1)->timeStart()!=musStreamList[i]->at(streamsIdx[i])->timeStart())
							streamsX[i] += (newElt->neededWidth());
						
						break;
					}
				}
				
				streamsIdx[i]++;
			}
			
		}
	}
}
