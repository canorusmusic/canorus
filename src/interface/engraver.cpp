/*! 
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QList>
#include <QMap>
#include <iostream>	//debug
#include "interface/engraver.h"

#include "widgets/scoreviewport.h"

#include "drawable/drawablestaff.h"
#include "drawable/drawableclef.h"
#include "drawable/drawablenote.h"
#include "drawable/drawableslur.h"
#include "drawable/drawablerest.h"
#include "drawable/drawablekeysignature.h"
#include "drawable/drawabletimesignature.h"
#include "drawable/drawablebarline.h"
#include "drawable/drawablemark.h"
#include "drawable/drawableaccidental.h"

#include "drawable/drawablelyricscontext.h"
#include "drawable/drawablesyllable.h"

#include "drawable/drawablefunctionmarkcontext.h"
#include "drawable/drawablefunctionmark.h"

#include "core/sheet.h"

#include "core/staff.h"
#include "core/voice.h"
#include "core/keysignature.h"
#include "core/timesignature.h"

#include "core/lyricscontext.h"
#include "core/syllable.h"

#include "core/functionmarkcontext.h"
#include "core/functionmark.h"

#define INITIAL_X_OFFSET 20 // space between the left border and the first music element
#define MINIMUM_SPACE 10    // minimum space between the music elements

QList<CADrawableMusElement*> CAEngraver::scalableElts;
int *CAEngraver::streamsRehersalMarks;

/*!
	\class CAEngraver
	\brief Class for correctly placing the abstract notes to the score canvas.
	
	This class is a bridge between the data part of Canorus and the UI.
	Out of data CAMusElement* and CAContext* objects, it creates their CADrawable* instances.
*/

/*!
	Repositions the notes in the abstract sheet of the given score viewport \a v so they fit nicely.
	This function doesn't clear the viewport, but only adds the elements.
*/
void CAEngraver::reposit( CAScoreViewPort *v ) {
	int i;
	CASheet *sheet = v->sheet();
	
	//list of all the music element lists (ie. streams) taken from all the contexts
	QList< QList<CAMusElement*> > musStreamList; // streams music elements
	QList<CAContext*> contexts; // which context does the stream belong to
	
	int dy = 50;
	QList<int> nonFirstVoiceIdxs;	//list of indexes of musStreamLists which the voices aren't the first voice. This is used later for determining should a sign be created or not (if it has been created in 1st voice already, don't recreate it in the other voices in the same staff).
	QMap<CAContext*, CADrawableContext*> drawableContextMap;
	
	for (int i=0; i < sheet->contextCount(); i++) {
		if (sheet->contextAt(i)->contextType() == CAContext::Staff) {
			if (i>0) dy+=70;
			
			CAStaff *staff = static_cast<CAStaff*>(sheet->contextAt(i));
			drawableContextMap[staff] = new CADrawableStaff(staff, 0, dy);
			v->addCElement(drawableContextMap[staff]);
			
			//add all the voices lists to the common list
			for (int j=0; j < staff->voiceCount(); j++) {
				musStreamList << staff->voiceAt(j)->musElementList();
				contexts << staff;
				if (staff->voiceAt(j)->voiceNumber()!=1)
					nonFirstVoiceIdxs << musStreamList.size()-1;
			}
			dy += drawableContextMap[staff]->height();
		} else
		if (sheet->contextAt(i)->contextType() == CAContext::LyricsContext) {
			CALyricsContext *lyricsContext = static_cast<CALyricsContext*>(sheet->contextAt(i));
			if (i>0 && (sheet->contextAt(i-1)->contextType() != CAContext::LyricsContext ||
			    static_cast<CALyricsContext*>(sheet->contextAt(i-1))->associatedVoice()->staff()!=lyricsContext->associatedVoice()->staff())) {
				dy+=70; // the previous context wasn't lyrics or was not related to the current lyrics
			}
			
			drawableContextMap[lyricsContext] = new CADrawableLyricsContext(lyricsContext, 0, dy);
			v->addCElement(drawableContextMap[lyricsContext]);
			QList<CAMusElement*> syllableList = lyricsContext->musElementList();
			musStreamList << syllableList;
			contexts << lyricsContext;
			dy += drawableContextMap[lyricsContext]->height();
		} else
		if (sheet->contextAt(i)->contextType() == CAContext::FunctionMarkContext) {
			if (i>0) dy+=70;
			
			CAFunctionMarkContext *fmContext = static_cast<CAFunctionMarkContext*>(sheet->contextAt(i));
			drawableContextMap[fmContext] = new CADrawableFunctionMarkContext(fmContext, 0, dy);
			v->addCElement(drawableContextMap[fmContext]);
			QList<CAFunctionMark*> fmList = fmContext->functionMarkList();
			QList<CAMusElement*> musList; for (int i=0; i<fmList.size(); i++) musList << fmList[i];
			musStreamList << musList;
			contexts << fmContext;
			dy += drawableContextMap[fmContext]->height();
		}
	}
	
	int streams = musStreamList.size();
	int streamsIdx[streams]; for (int i=0; i<streams; i++) streamsIdx[i] = 0;
	int streamsX[streams]; for (int i=0; i<streams; i++) streamsX[i] = INITIAL_X_OFFSET;
	int streamsRehersalMarks[streams]; for (int i=0; i<streams; i++) streamsRehersalMarks[i] = 0;
	CAEngraver::streamsRehersalMarks = streamsRehersalMarks;
	CAClef *lastClef[streams]; for (int i=0; i<streams; i++) lastClef[i] = 0;
	CAKeySignature *lastKeySig[streams]; for (int i=0; i<streams; i++) lastKeySig[i] = 0;
	CATimeSignature *lastTimeSig[streams]; for (int i=0; i<streams; i++) lastTimeSig[i] = 0;	
	scalableElts.clear();
	
	int timeStart = 0;
	bool done = false;
	CADrawableFunctionMarkSupport *lastDFMTonicizations[streams]; for (int i=0; i<streams; i++) lastDFMTonicizations[i]=0;
	while (!done) {
		//if all the indices are at the end of the streams, finish.
		int idx;
		for (idx=0; (idx < streams) && (streamsIdx[idx] == musStreamList[idx].size()); idx++);
		if (idx==streams) { done=true; continue; }
		
		//Synchronize minimum X-es between the contexts
		int maxX = 0;		
		for (int i=0; i<streams; i++) maxX = (streamsX[i] > maxX) ? streamsX[i] : maxX;
		for (int i=0; i<streams; i++)
			if (musStreamList[i].size() &&
			    musStreamList[i].last()->musElementType()!=CAMusElement::FunctionMark)
				streamsX[i] = maxX;
		
		//go through all the streams and remember the time of the soonest element that will happen
		timeStart = -1;	//reset the timeStart - the next one minimum will be set in the following loop
		for (int idx=0; idx < streams; idx++) {
			if ( (musStreamList[idx].size() > streamsIdx[idx]) && ((timeStart==-1) || (musStreamList[idx].at(streamsIdx[idx])->timeStart() < timeStart)) )
				timeStart = musStreamList[idx].at(streamsIdx[idx])->timeStart();
		}
		//timeStart now holds the nearest next time we're going to draw
		
		//go through all the streams and check if the following element has this time
		CAMusElement *elt;
		CADrawableContext *drawableContext;
		bool placedSymbol = false;	//used if waiting for notes to gather and a non-time-consuming symbol has been placed
		for (int i=0; i < streams; i++) {
			//loop until the first playable element
			//multiple elements can have the same start time. eg. Clef + Key signature + Time signature + First note
			while ( (streamsIdx[i] < musStreamList[i].size()) &&
			        ((elt = musStreamList[i].at(streamsIdx[i]))->timeStart() == timeStart) &&
			        (!elt->isPlayable()) &&
			        (elt->musElementType() != CAMusElement::Barline) &&	//barlines should be aligned
			        (elt->musElementType() != CAMusElement::FunctionMark) &&	//function marks are placed separately
			        (elt->musElementType() != CAMusElement::Syllable)	//syllables are placed separately
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
							
							// set the last clefs in all voices in the same staff
							for (int j=0; j<contexts.size(); j++)
								if (contexts[j]==contexts[i])
									lastClef[j] = clef->clef();
							
							streamsX[i] += (clef->neededWidth() + MINIMUM_SPACE);
							placedSymbol = true;
							
							placeMarks( clef, v, i );
							
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
							
							// set the last key sigs in all voices in the same staff
							for (int j=0; j<contexts.size(); j++)
								if (contexts[j]==contexts[i])
									lastKeySig[j] = keySig->keySignature();
							
							streamsX[i] += (keySig->neededWidth() + MINIMUM_SPACE);
							placedSymbol = true;
							
							placeMarks( keySig, v, i );
							
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
							
							// set the last time signatures in all voices in the same staff
							for (int j=0; j<contexts.size(); j++)
								if (contexts[j]==contexts[i])
									lastTimeSig[j] = timeSig->timeSignature();
							
							streamsX[i] += (timeSig->neededWidth() + MINIMUM_SPACE);
							placedSymbol = true;
							
							placeMarks( timeSig, v, i );
							
							break;
						}
					} /* SWITCH */
					
				} /* IF firstVoice */
				streamsIdx[i]++;
			}
		}
		
		// Draw function key name, if needed
		QList<CADrawableFunctionMarkSupport*> lastDFMKeyNames;
		for (int i=0;
		     (i<streams) &&
		     (streamsIdx[i] < musStreamList[i].size()) &&
			 ((elt = musStreamList[i].at(streamsIdx[i]))->timeStart() == timeStart);
			 i++) {
			CAMusElement *elt = musStreamList[i].at(streamsIdx[i]);
			if (elt->musElementType()==CAMusElement::FunctionMark && ((CAFunctionMark*)elt)->function()!=CAFunctionMark::Undefined) {
				drawableContext = drawableContextMap[elt->context()];
				if (streamsIdx[i]-1<0 ||
				    ((CAFunctionMark*)musStreamList[i].at(streamsIdx[i]-1))->key() != ((CAFunctionMark*)elt)->key()
				   ) {
					//draw new function mark key, if it was changed or if it's the first function in the score
					CADrawableFunctionMarkSupport *support = new CADrawableFunctionMarkSupport(
						CADrawableFunctionMarkSupport::Key,
						((CAFunctionMark*)elt)->key(),
						drawableContext,
						streamsX[i],
						((CADrawableFunctionMarkContext*)drawableContext)->yPosLine(CADrawableFunctionMarkContext::Middle)
					);
					streamsX[i] += (support->neededWidth());
					v->addMElement(support);
					lastDFMKeyNames << support;
				}
			}
		}
		
		// Synchronize minimum X-es between the contexts - all the noteheads or barlines should be horizontally aligned.
		for (int i=0; i<streams; i++) maxX = (streamsX[i] > maxX) ? streamsX[i] : maxX;
		for (int i=0; i<streams; i++)
			if (musStreamList[i].size() &&
			    musStreamList[i].last()->musElementType()!=CAMusElement::FunctionMark)
				streamsX[i] = maxX;
		
		// Place barlines
		for (int i=0; i<streams; i++) {
			if (!(musStreamList[i].size() > streamsIdx[i]) ||	//if the stream is already at the end, continue to the next stream
			    ((elt = musStreamList[i].at(streamsIdx[i]))->timeStart() != timeStart))
				continue;
			
			if ((elt = musStreamList[i].at(streamsIdx[i]))->musElementType() == CAMusElement::Barline) {
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
				
				placeMarks( bar, v, i );
			}
		}
		
		// Place accidentals and key names of the function marks, if needed.
		// These elements are so called Support elements. They can't be selected and they're not really connected usually to any logical element, but they're needed when drawing.
		int maxWidth = 0;
		int maxAccidentalXEnd = 0;
		QList<CADrawableAccidental*> lastAccidentals; 
		for (int i=0; i < streams; i++) {
			// loop until the element has come, which has bigger timeStart
			CADrawableMusElement *newElt=0;
			int oldStreamIdx = streamsIdx[i];
			while ( (streamsIdx[i] < musStreamList[i].size()) &&
			        ((elt = musStreamList[i].at(streamsIdx[i]))->timeStart() == timeStart) &&
			        (elt->isPlayable())
			      ) {
				drawableContext = drawableContextMap[elt->context()];
				
				if (elt->musElementType()==CAMusElement::Note &&
				    ((CADrawableStaff*)drawableContext)->getAccs(streamsX[i], static_cast<CANote*>(elt)->diatonicPitch().noteName()) != static_cast<CANote*>(elt)->diatonicPitch().accs()
				   ) {
						newElt = new CADrawableAccidental(
							((CANote*)elt)->diatonicPitch().accs(),
							((CANote*)elt),
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
			
			streamsX[i] += (maxWidth?maxWidth+1:0);	// append the needed space for the last used note
		}
		
		// Synchronize minimum X-es between the contexts - all the noteheads or barlines should be horizontally aligned.
		for (int i=0; i<streams; i++) maxX = (streamsX[i] > maxX) ? streamsX[i] : maxX;
		for (int i=0; i<streams; i++) streamsX[i] = maxX;
		
		// Align support elements (accidentals, function key names) to the right
		for (int i=0; i<lastDFMKeyNames.size(); i++)
			lastDFMKeyNames[i]->setXPos(maxX - lastDFMKeyNames[i]->neededWidth() - 2);
		
		int deltaXPos = maxX - maxAccidentalXEnd;
		for (int i=0; i<lastAccidentals.size(); i++) {
			lastAccidentals[i]->setXPos(lastAccidentals[i]->xPos()+deltaXPos-1);
		}
		
		// Place noteheads and other elements aligned to noteheads (syllables, function marks)
		for (int i=0; i < streams; i++) {
			// loop until the element has come, which has bigger timeStart
			while ( (streamsIdx[i] < musStreamList[i].size()) &&
			        ((elt = musStreamList[i].at(streamsIdx[i]))->timeStart() == timeStart) &&
			        (elt->isPlayable() ||
			         elt->musElementType()==CAMusElement::FunctionMark ||
			         elt->musElementType()==CAMusElement::Syllable
			        )
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
						
						// Create Ties
						if ( static_cast<CADrawableNote*>(newElt)->note()->tieStart() ) {
							CASlur::CASlurDirection dir = static_cast<CADrawableNote*>(newElt)->note()->tieStart()->slurDirection();
							if ( dir==CASlur::SlurPreferred || dir==CASlur::SlurNeutral )
								dir = static_cast<CADrawableNote*>(newElt)->note()->actualSlurDirection();
							CADrawableSlur *tie=0;
							if (dir==CASlur::SlurUp) {
								tie = new CADrawableSlur(
									static_cast<CADrawableNote*>(newElt)->note()->tieStart(), drawableContext,
									newElt->xPos()+newElt->width(), newElt->yPos(),
									newElt->xPos() + 20, newElt->yPos() - 5,
									newElt->xPos() + 40, newElt->yPos()								
								);
							} else
							if (dir==CASlur::SlurDown) {
								tie = new CADrawableSlur(
									static_cast<CADrawableNote*>(newElt)->note()->tieStart(), drawableContext,
									newElt->xPos()+newElt->width(), newElt->yPos() + newElt->height(),
									newElt->xPos() + 20, newElt->yPos() + newElt->height() + 5,
									newElt->xPos() + 40, newElt->yPos()	+ newElt->height()							
								);
							}
							v->addMElement(tie);
						}
						if ( static_cast<CADrawableNote*>(newElt)->note()->tieEnd() ) {
							// Set the slur coordinates for the second note
							CASlur::CASlurDirection dir = static_cast<CADrawableNote*>(newElt)->note()->tieEnd()->slurDirection();
							if ( dir==CASlur::SlurPreferred || dir==CASlur::SlurNeutral )
								dir = static_cast<CADrawableNote*>(newElt)->note()->tieEnd()->noteStart()->actualSlurDirection();
							CADrawableSlur *dSlur = static_cast<CADrawableSlur*>(v->findMElement(static_cast<CADrawableNote*>(newElt)->note()->tieEnd()));
							dSlur->setX2( newElt->xPos() );
							dSlur->setXMid( qRound(0.5*dSlur->xPos() + 0.5*newElt->xPos()) );
							if ( dir==CASlur::SlurUp ) {
								dSlur->setY2( newElt->yPos() );
								dSlur->setYMid( qMin( dSlur->y2(), dSlur->y1() ) - 5 );
							} else
							if ( dir==CASlur::SlurDown ) {
								dSlur->setY2( newElt->yPos() + newElt->height() );
								dSlur->setYMid( qMax( dSlur->y2(), dSlur->y1() ) + 5 );
							}

						}
						
						// Create Slurs
						if ( static_cast<CADrawableNote*>(newElt)->note()->slurStart() ) {
							CASlur::CASlurDirection dir = static_cast<CADrawableNote*>(newElt)->note()->slurStart()->slurDirection();
							if ( dir==CASlur::SlurPreferred || dir==CASlur::SlurNeutral )
								dir = static_cast<CADrawableNote*>(newElt)->note()->actualSlurDirection();
							CADrawableSlur *slur=0;
							if (dir==CASlur::SlurUp) {
								slur = new CADrawableSlur(
									static_cast<CADrawableNote*>(newElt)->note()->slurStart(), drawableContext,
									newElt->xPos()+newElt->width(), newElt->yPos(),
									newElt->xPos() + 20, newElt->yPos() - 15,
									newElt->xPos() + 40, newElt->yPos()								
								);
							} else
							if (dir==CASlur::SlurDown) {
								slur = new CADrawableSlur(
									static_cast<CADrawableNote*>(newElt)->note()->slurStart(), drawableContext,
									newElt->xPos()+newElt->width(), newElt->yPos() + newElt->height(),
									newElt->xPos() + 20, newElt->yPos() + newElt->height() + 15,
									newElt->xPos() + 40, newElt->yPos()	+ newElt->height()							
								);
							}
							v->addMElement(slur);
						}
						if ( static_cast<CADrawableNote*>(newElt)->note()->slurEnd() ) {
							// Set the slur coordinates for the second note
							CASlur::CASlurDirection dir = static_cast<CADrawableNote*>(newElt)->note()->slurEnd()->slurDirection();
							if ( dir==CASlur::SlurPreferred || dir==CASlur::SlurNeutral )
								dir = static_cast<CADrawableNote*>(newElt)->note()->slurEnd()->noteStart()->actualSlurDirection();
							CADrawableSlur *dSlur = static_cast<CADrawableSlur*>(v->findMElement(static_cast<CADrawableNote*>(newElt)->note()->slurEnd()));
							dSlur->setX2( newElt->xPos() );
							dSlur->setXMid( qRound(0.5*dSlur->xPos() + 0.5*newElt->xPos()) );
							if ( dir==CASlur::SlurUp ) {
								dSlur->setY2( newElt->yPos() );
								dSlur->setYMid( qMin( dSlur->y2(), dSlur->y1() ) - 15 );
							} else
							if ( dir==CASlur::SlurDown ) {
								dSlur->setY2( newElt->yPos() + newElt->height() );
								dSlur->setYMid( qMax( dSlur->y2(), dSlur->y1() ) + 15 );
							}
						}
						
						// Create Phrasing Slurs
						if ( static_cast<CADrawableNote*>(newElt)->note()->phrasingSlurStart() ) {
							CASlur::CASlurDirection dir = static_cast<CADrawableNote*>(newElt)->note()->phrasingSlurStart()->slurDirection();
							if ( dir==CASlur::SlurPreferred || dir==CASlur::SlurNeutral)
								dir = static_cast<CADrawableNote*>(newElt)->note()->actualSlurDirection();
							CADrawableSlur *phrasingSlur=0;
							if (dir==CASlur::SlurUp) {
								phrasingSlur = new CADrawableSlur(
									static_cast<CADrawableNote*>(newElt)->note()->phrasingSlurStart(), drawableContext,
									newElt->xPos()+newElt->width(), newElt->yPos(),
									newElt->xPos() + 20, newElt->yPos() - 19,
									newElt->xPos() + 40, newElt->yPos()								
								);
							} else
							if (dir==CASlur::SlurDown) {
								phrasingSlur = new CADrawableSlur(
									static_cast<CADrawableNote*>(newElt)->note()->phrasingSlurStart(), drawableContext,
									newElt->xPos()+newElt->width(), newElt->yPos() + newElt->height(),
									newElt->xPos() + 20, newElt->yPos() + newElt->height() + 19,
									newElt->xPos() + 40, newElt->yPos()	+ newElt->height()							
								);
							}
							v->addMElement(phrasingSlur);
						}
						if ( static_cast<CADrawableNote*>(newElt)->note()->phrasingSlurEnd() ) {
							// Set the slur coordinates for the second note
							CASlur::CASlurDirection dir = static_cast<CADrawableNote*>(newElt)->note()->phrasingSlurEnd()->slurDirection();
							if ( dir==CASlur::SlurPreferred || dir==CASlur::SlurNeutral )
								dir = static_cast<CADrawableNote*>(newElt)->note()->phrasingSlurEnd()->noteStart()->actualSlurDirection();
							CADrawableSlur *dSlur = static_cast<CADrawableSlur*>(v->findMElement(static_cast<CADrawableNote*>(newElt)->note()->phrasingSlurEnd()));
							dSlur->setX2( newElt->xPos() );
							dSlur->setXMid( qRound(0.5*dSlur->xPos() + 0.5*newElt->xPos()) );
							if ( dir==CASlur::SlurUp ) {
								dSlur->setY2( newElt->yPos() );
								dSlur->setYMid( qMin( dSlur->y2(), dSlur->y1() ) - 19 );
							} else
							if ( dir==CASlur::SlurDown ) {
								dSlur->setY2( newElt->yPos() + newElt->height() );
								dSlur->setYMid( qMax( dSlur->y2(), dSlur->y1() ) + 19 );
							}
						}
						
						v->addMElement(newElt);
						if ( static_cast<CANote*>(elt)->isLastInTheChord() )	
							streamsX[i] += (newElt->neededWidth() + MINIMUM_SPACE);
						
						placeMarks( newElt, v, i );
						
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
						
						placeMarks( newElt, v, i );
						
						break;
					}
					case CAMusElement::Syllable: {
						newElt = new CADrawableSyllable(
							static_cast<CASyllable*>(elt),
							static_cast<CADrawableLyricsContext*>(drawableContext),
							streamsX[i],
							drawableContext->yPos() + qRound(CADrawableLyricsContext::DEFAULT_TEXT_VERTICAL_SPACING)
						);
						
						CAMusElement *prevSyllable = drawableContext->context()->previous(elt);
						CADrawableMusElement *prevDSyllable = (prevSyllable?v->findMElement(prevSyllable):0);
						if (prevDSyllable) {
							prevDSyllable->setWidth( newElt->xPos() - prevDSyllable->xPos() );
						}
						
						v->addMElement(newElt);
						streamsX[i] += (newElt->neededWidth() + MINIMUM_SPACE);
						break;
					}
					case CAMusElement::FunctionMark: {
						CAFunctionMark *function = static_cast<CAFunctionMark*>(elt);
						
						// Make a new line, if parallel function present
						if (streamsIdx[i]-1>=0 && musStreamList[i].at(streamsIdx[i]-1)->timeStart()==musStreamList[i].at(streamsIdx[i])->timeStart()) {
							((CADrawableFunctionMarkContext*)drawableContext)->nextLine();
							CADrawableFunctionMarkSupport *newKey = new CADrawableFunctionMarkSupport(
								CADrawableFunctionMarkSupport::Key,
								function->key(),
								drawableContext,
								streamsX[i],
								((CADrawableFunctionMarkContext*)drawableContext)->yPosLine(CADrawableFunctionMarkContext::Middle)
							);
							newKey->setXPos(streamsX[i]-newKey->width()-2);
							v->addMElement(newKey);
						}
						
						// Place the function itself, if it's independent
						newElt = new CADrawableFunctionMark(
							function,
							static_cast<CADrawableFunctionMarkContext*>(drawableContext),
							streamsX[i],
							(function->tonicDegree()==CAFunctionMark::T && (!function->isPartOfEllipse()))?
								static_cast<CADrawableFunctionMarkContext*>(drawableContext)->yPosLine(CADrawableFunctionMarkContext::Middle):
								static_cast<CADrawableFunctionMarkContext*>(drawableContext)->yPosLine(CADrawableFunctionMarkContext::Upper)
						);
						
						// Place alterations
						CADrawableFunctionMarkSupport *alterations=0;
						if (function->addedDegrees().size() || function->alteredDegrees().size()) {
							alterations = new CADrawableFunctionMarkSupport(
								CADrawableFunctionMarkSupport::Alterations,
								function,
								drawableContext,
								streamsX[i],
								((CADrawableFunctionMarkContext*)drawableContext)->yPosLine(CADrawableFunctionMarkContext::Lower)+3
							);
							// center-align alterations to function, if placed
							if (newElt)
								alterations->setXPos((int)(newElt->xPos()+newElt->width()/2.0-alterations->width()/2.0+0.5));
							else	//center-align to note
								alterations->setXPos((int)(streamsX[i]+5-alterations->width()/2.0+0.5));								
						}
						
						// Place tonicization. The same tonicization is always placed from streamsIdx[i]-nth to streamsIdx[i]-1th element, where streamsIdx[i] is the current index. 
						int j=streamsIdx[i]-1;	//index of the previous elt
						CADrawableFunctionMarkSupport *tonicization=0;
						for (; j>=0 && ((CAFunctionMark*)musStreamList[i].at(j))->function()==CAFunctionMark::Undefined; j--);	//ignore any alterations back there
						if (j>=0 && (
						    // place tonicization, if tonic degree is not default
						    ((CAFunctionMark*)musStreamList[i].at(j))->tonicDegree()!=CAFunctionMark::T &&
						    ((CAFunctionMark*)musStreamList[i].at(j))->tonicDegree()!=CAFunctionMark::Undefined
							// and it's not still the same
							&& (function->tonicDegree()!=((CAFunctionMark*)musStreamList[i].at(j))->tonicDegree() || function->key()!=((CAFunctionMark*)musStreamList[i].at(j))->key())
						    // always place tonicization, if ellipse is present
						    //|| ((CAFunctionMark*)musStreamList[i].at(j))->isPartOfEllipse()
						)) {
							CAFunctionMark::CAFunctionType type = ((CAFunctionMark*)musStreamList[i].at(j))->tonicDegree();
							CAFunctionMark *right = (CAFunctionMark*)musStreamList[i].at(j);
							
							// find the n-th element back
							while (--j>=0 && ((CAFunctionMark*)musStreamList[i].at(j))->tonicDegree()==((CAFunctionMark*)musStreamList[i].at(j+1))->tonicDegree() &&
							       ((CAFunctionMark*)musStreamList[i].at(j))->key()==((CAFunctionMark*)musStreamList[i].at(j+1))->key());
							CAFunctionMark *tonicStart = (CAFunctionMark*)musStreamList[i].at(++j);
							CADrawableFunctionMark *left = (CADrawableFunctionMark*)drawableContext->findMElement(tonicStart);
							if (tonicStart!=(CAMusElement*)musStreamList[i].at(streamsIdx[i]-1)) {	// tonicization isn't single (more than 1 tonic element)
								tonicization = new CADrawableFunctionMarkSupport(
									CADrawableFunctionMarkSupport::Tonicization,
									left,
									(CADrawableFunctionMarkContext*)drawableContext,
									left->xPos(),
									((CADrawableFunctionMarkContext*)drawableContext)->yPosLine(CADrawableFunctionMarkContext::Middle),
									(CADrawableFunctionMark*)drawableContext->findMElement(right)
								);
							} else {																// tonicization is single (one tonic)
								tonicization = new CADrawableFunctionMarkSupport(
									CADrawableFunctionMarkSupport::Tonicization,
									left,
									(CADrawableFunctionMarkContext*)drawableContext,
									left->xPos(),
									((CADrawableFunctionMarkContext*)drawableContext)->yPosLine(CADrawableFunctionMarkContext::Middle)
								);
								tonicization->setXPos((int)(left->xPos()+0.5*left->width()-0.5*tonicization->width()+0.5));	// align center
							}
						}
						
						// Place horizontal modulation rectangle, if needed
						j=streamsIdx[i]-1;
						CADrawableFunctionMarkSupport *hModulationRect=0;
						if (newElt && j>=0) {
							if (((CAFunctionMark*)musStreamList[i].at(j))->key()!=function->key() &&
							    ((CAFunctionMark*)musStreamList[i].at(j))->timeStart()!=function->timeStart()) {
								CADrawableFunctionMark *left = (CADrawableFunctionMark*)drawableContext->findMElement(musStreamList[i].at(j));
								hModulationRect = new CADrawableFunctionMarkSupport(
									CADrawableFunctionMarkSupport::Rectangle,
									left,
									(CADrawableFunctionMarkContext*)drawableContext,
									left->xPos(),
									left->yPos(),
									(CADrawableFunctionMark*)newElt
								);
								if (streamsIdx[i]%2)
									hModulationRect->setRectWider(true);	//make it wider, so it potentially doesn't overlap with other rectangles around
						    }
						}
						
						// Place vertical modulation rectangle, if needed
						// This must be done *before* the extender lines are placed!
						j=streamsIdx[i]-1;
						CADrawableFunctionMarkSupport *vModulationRect=0;
						while (--j>=0 &&
						       ((CAFunctionMark*)musStreamList[i].at(j))->key()!=((CAFunctionMark*)musStreamList[i].at(j+1))->key() &&
						       ((CAFunctionMark*)musStreamList[i].at(j))->timeStart()==((CAFunctionMark*)musStreamList[i].at(j+1))->timeStart()
						      );
						if (++j>=0 && j!=streamsIdx[i]-1) {
							CADrawableFunctionMark *left = (CADrawableFunctionMark*)drawableContext->findMElement(musStreamList[i].at(j));
							vModulationRect = new CADrawableFunctionMarkSupport(
								CADrawableFunctionMarkSupport::Rectangle,
								left,
								(CADrawableFunctionMarkContext*)drawableContext,
								left->xPos(),
								left->yPos(),
								(CADrawableFunctionMark*)drawableContext->findMElement(musStreamList[i].at(streamsIdx[i]-1))
							);
						}
						
						// Place horizontal chord area rectangle for the previous elements, if element neighbours are of the same chordarea/function
						j=streamsIdx[i]-1;
						CADrawableFunctionMarkSupport *hChordAreaRect=0;
						if (j>=0 && // don't draw rectangle, if the current element would still be in the rectangle
						    (
						     ((CAFunctionMark*)musStreamList[i].at(j))->key()==function->key() && ((CAFunctionMark*)musStreamList[i].at(j))->function()!=function->function() && ((CAFunctionMark*)musStreamList[i].at(j))->function()!=function->chordArea() && ((CAFunctionMark*)musStreamList[i].at(j))->chordArea()!=function->chordArea()
						     || ((CAFunctionMark*)musStreamList[i].at(j))->key()!=function->key()
						     || j==musStreamList[i].size()
						    )
						   ) {
							bool oneFunctionOnly=true;
							while (--j>=0 &&
							       ((CAFunctionMark*)musStreamList[i].at(j+1))->chordArea()!=CAFunctionMark::Undefined &&
							       ((CAFunctionMark*)musStreamList[i].at(j))->key()==((CAFunctionMark*)musStreamList[i].at(j+1))->key() &&
							        (((CAFunctionMark*)musStreamList[i].at(j))->chordArea()==((CAFunctionMark*)musStreamList[i].at(j+1))->chordArea() ||
							         ((CAFunctionMark*)musStreamList[i].at(j))->function()==((CAFunctionMark*)musStreamList[i].at(j+1))->chordArea() ||
							         ((CAFunctionMark*)musStreamList[i].at(j))->chordArea()==((CAFunctionMark*)musStreamList[i].at(j+1))->function()
							        )
							      )
								if ( ((CAFunctionMark*)musStreamList[i].at(j))->function()!=((CAFunctionMark*)musStreamList[i].at(j+1))->function() )
									oneFunctionOnly = false;
							
							if ( ++j != streamsIdx[i]-1 && !oneFunctionOnly ) {
								CADrawableFunctionMark *left = (CADrawableFunctionMark*)drawableContext->findMElement(musStreamList[i].at(j));
								hChordAreaRect = new CADrawableFunctionMarkSupport(
									CADrawableFunctionMarkSupport::Rectangle,
									left,
									(CADrawableFunctionMarkContext*)drawableContext,
									left->xPos(),
									left->yPos(),
									(CADrawableFunctionMark*)drawableContext->findMElement(musStreamList[i].at(streamsIdx[i]-1))
								);
							}
						}
						
						// Place chordarea mark below in paranthesis, if no neighbours are of same chordarea
						CADrawableFunctionMarkSupport *chordArea=0;
						j=streamsIdx[i];
						if (newElt &&
						    function->chordArea()!=CAFunctionMark::Undefined &&
						    function->chordArea()!=function->function() && // chord area is the same as function name - don't draw chordarea then 
						    (j-1<0 ||
						     (((CAFunctionMark*)musStreamList[i].at(j-1))->key()==function->key() &&
						      ((CAFunctionMark*)musStreamList[i].at(j-1))->chordArea()!=function->chordArea() &&
						      ((CAFunctionMark*)musStreamList[i].at(j-1))->function()!=function->chordArea() &&
						      ((CAFunctionMark*)musStreamList[i].at(j-1))->chordArea()!=function->function() ||
						      ((CAFunctionMark*)musStreamList[i].at(j-1))->tonicDegree()!=function->tonicDegree() ||
						      ((CAFunctionMark*)musStreamList[i].at(j-1))->key()!=function->key()
						     )
						    ) &&
						    (j+1>=musStreamList[i].size() ||
						     (((CAFunctionMark*)musStreamList[i].at(j+1))->key()==function->key() &&
						      ((CAFunctionMark*)musStreamList[i].at(j+1))->chordArea()!=function->chordArea() &&
						      ((CAFunctionMark*)musStreamList[i].at(j+1))->function()!=function->chordArea() &&
						      ((CAFunctionMark*)musStreamList[i].at(j+1))->chordArea()!=function->function() ||
						      ((CAFunctionMark*)musStreamList[i].at(j+1))->tonicDegree()!=function->tonicDegree() ||
						      ((CAFunctionMark*)musStreamList[i].at(j+1))->key()!=function->key()
						     )
						    )
						   ) {
							chordArea = new CADrawableFunctionMarkSupport(
								CADrawableFunctionMarkSupport::ChordArea,
								(CADrawableFunctionMark*)newElt,
								(CADrawableFunctionMarkContext*)drawableContext,
								streamsX[i],
								((CADrawableFunctionMarkContext*)drawableContext)->yPosLine(CADrawableFunctionMarkContext::Lower)								
							);
							chordArea->setXPos((int)(newElt->xPos()-(chordArea->width()-newElt->width())/2.0 + 0.5));
						}
						
						// Place ellipse
						j=streamsIdx[i]-1;
						CADrawableFunctionMarkSupport *ellipse=0;
						if (j>=0 && ((CAFunctionMark*)musStreamList[i].at(j))->isPartOfEllipse()	//place ellipse, if it has it
							&& (!function->isPartOfEllipse()) ) {	//and it's not lasting anymore
							//find the n-th element back
							while (--j>=0 && ((CAFunctionMark*)musStreamList[i].at(j))->isPartOfEllipse());
							CAFunctionMark *ellipseStart = (CAFunctionMark*)musStreamList[i].at(++j);
							CADrawableFunctionMark *left = (CADrawableFunctionMark*)drawableContext->findMElement(ellipseStart);
							ellipse = new CADrawableFunctionMarkSupport(
								CADrawableFunctionMarkSupport::Ellipse,
								left,
								(CADrawableFunctionMarkContext*)drawableContext,
								left->xPos(),
								((CADrawableFunctionMarkContext*)drawableContext)->yPosLine(CADrawableFunctionMarkContext::Lower),
								(CADrawableFunctionMark*)drawableContext->findMElement((CAMusElement*)musStreamList[i].at(streamsIdx[i]-1))
							);
						}
						
						// Set extender line and change the width of the previous function mark
						if ( newElt && streamsIdx[i]-1>=0 && musStreamList[i].at(streamsIdx[i]-1)->timeStart()!=musStreamList[i].at(streamsIdx[i])->timeStart() ) {
							CAFunctionMark *prevElt = static_cast<CAFunctionMark*>(musStreamList[i].at(streamsIdx[i]-1));
							CADrawableFunctionMark *prevDFM =
								static_cast<CADrawableFunctionMark*>(drawableContext->findMElement(prevElt));
							
							// draw extender line instead of the function, if functions are the same
							if ( function->function()!=CAFunctionMark::Undefined &&
							     function->function()==prevElt->function() &&
							     function->tonicDegree()==prevElt->tonicDegree() &&
							     function->key()==prevElt->key() ) {
								static_cast<CADrawableFunctionMark*>(newElt)->setExtenderLineOnly( true );
								static_cast<CADrawableFunctionMark*>(newElt)->setExtenderLineVisible( true );
								prevDFM->setExtenderLineVisible( true );
								
								if (tonicization) {
									//tonicization->setExtenderLineVisible( true );
									if ( prevElt->key()==function->key() )
										tonicization->setWidth( newElt->xPos()-tonicization->xPos() );
									else
										tonicization->setWidth( lastDFMKeyNames[i]->xPos()-tonicization->xPos() );
								}
								
								if (lastDFMTonicizations[i]) {
									//lastDFMTonicizations[i]->setExtenderLineVisible(true);
									if ( prevElt->key()==function->key() )
										lastDFMTonicizations[i]->setWidth( newElt->xPos()-lastDFMTonicizations[i]->xPos() );
									else
										lastDFMTonicizations[i]->setWidth( lastDFMKeyNames[i]->xPos()-lastDFMTonicizations[i]->xPos() );
								}
							}
							
							if ( prevDFM && prevDFM->isExtenderLineVisible() ) {
								if ( prevElt->key()==function->key() )
									prevDFM->setWidth( newElt->xPos()-prevDFM->xPos() );
								else
									prevDFM->setWidth( lastDFMKeyNames[i]->xPos()-prevDFM->xPos() );
							}
						}
						
						if (newElt) v->addMElement(newElt);	// when only alterations are made and no function placed, IF is needed
						if (tonicization) { v->addMElement(tonicization); lastDFMTonicizations[i]=tonicization; }
						if (ellipse) v->addMElement(ellipse);
						if (hModulationRect) v->addMElement(hModulationRect);
						if (vModulationRect) v->addMElement(vModulationRect);
						if (hChordAreaRect) v->addMElement(hChordAreaRect);
						if (chordArea) v->addMElement(chordArea);
						if (alterations) v->addMElement(alterations);
						
						if (newElt && streamsIdx[i]+1<musStreamList[i].size() && musStreamList[i].at(streamsIdx[i]+1)->timeStart()!=musStreamList[i].at(streamsIdx[i])->timeStart())
							streamsX[i] += (newElt->neededWidth());
						
						break;
					}
				}
				
				streamsIdx[i]++;
			}
			
		}
	}
	
	// reposit the scalable elements (eg. crescendo)
	for (int i=0; i<scalableElts.size(); i++) {
		scalableElts[i]->setXPos( v->timeToCoords(scalableElts[i]->musElement()->timeStart()) );
		scalableElts[i]->setWidth( v->timeToCoords(scalableElts[i]->musElement()->timeEnd()) - scalableElts[i]->xPos() );
		v->addMElement(scalableElts[i]);
	}
}

/*!
	Place marks for the given music element.
*/
void CAEngraver::placeMarks( CADrawableMusElement *e, CAScoreViewPort *v, int streamIdx ) {
	CAMusElement *elt = e->musElement();
	int xCoord = e->xPos();
	
	for ( int i=0,j=0,k=0; i < elt->markList().size(); i++ ) {
		if ( elt->markList()[i]->isCommon() &&
		     elt->musElementType()==CAMusElement::Note &&
		     !static_cast<CANote*>(elt)->isFirstInTheChord() ) {
			continue;
		}
		
		CAMark *mark = elt->markList()[i];
		
		int yCoord;
		if ( mark->markType()==CAMark::Pedal ||
		     ( mark->markType()==CAMark::Fingering && (static_cast<CAFingering*>(mark)->fingerList()[0]==CAFingering::LHeel || static_cast<CAFingering*>(mark)->fingerList()[0]==CAFingering::LToe )) ||
		     elt->musElementType()==CAMusElement::Note && static_cast<CANote*>(elt)->actualSlurDirection()==CASlur::SlurDown && 
		     (mark->markType()==CAMark::Fermata || mark->markType()==CAMark::Articulation || mark->markType()==CAMark::Fingering && static_cast<CAFingering*>(mark)->fingerList()[0]!=CAFingering::LHeel && static_cast<CAFingering*>(mark)->fingerList()[0]!=CAFingering::LToe ))  {
			// place mark below
			xCoord = e->xPos();
			yCoord = qMax(e->yPos()+e->height(),e->drawableContext()->yPos()+e->drawableContext()->height())+20*(k+1);
			k++;
		} else if ( elt->musElementType()==CAMusElement::Note &&
				    static_cast<CANote*>(elt)->isPartOfTheChord() &&
		            mark->markType()==CAMark::Fingering && static_cast<CAFingering*>(mark)->fingerList()[0] < 6 ) {
			// place mark beside the note
			xCoord = e->xPos() + e->width();
			yCoord = e->yPos() - 2;
		} else {
			// place mark above
			xCoord = e->xPos();
			yCoord = qMin(e->yPos(),e->drawableContext()->yPos())-20*(j+1);
			j++;
		}
		
		if ( mark->markType()==CAMark::Articulation ) {
			xCoord = e->xPos() + qRound(e->width()/2.0) - 3;
		}
		
		CADrawableMark *m = new CADrawableMark( mark, e->drawableContext(), xCoord, yCoord );
		
		if ( mark->markType()==CAMark::RehersalMark )
			m->setRehersalMarkNumber( streamsRehersalMarks[ streamIdx ]++ );
		
		if (m->isHScalable() || m->isVScalable()) {
			scalableElts << m;
		} else {
			v->addMElement( m );
		}
	}
}
