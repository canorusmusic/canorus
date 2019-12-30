/*!
    Copyright (c) 2006-2019, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QList>
#include <QMap>
#include <QDebug>

#include "layout/layoutengine.h"

#include "widgets/scoreview.h"

#include "layout/drawablestaff.h"
#include "layout/drawableclef.h"
#include "layout/drawablenote.h"
#include "layout/drawableslur.h"
#include "layout/drawabletuplet.h"
#include "layout/drawablerest.h"
#include "layout/drawablemidinote.h"
#include "layout/drawablekeysignature.h"
#include "layout/drawabletimesignature.h"
#include "layout/drawablebarline.h"
#include "layout/drawablemark.h"
#include "layout/drawableaccidental.h"
#include "layout/drawablenotecheckererror.h"

#include "layout/drawablelyricscontext.h"
#include "layout/drawablesyllable.h"

#include "layout/drawablefiguredbasscontext.h"
#include "layout/drawablefiguredbassnumber.h"

#include "layout/drawablefunctionmarkcontext.h"
#include "layout/drawablefunctionmark.h"

#include "layout/drawablechordnamecontext.h"
#include "layout/drawablechordname.h"

#include "score/sheet.h"

#include "score/staff.h"
#include "score/voice.h"
#include "score/keysignature.h"
#include "score/timesignature.h"

#include "score/note.h"
#include "score/rest.h"
#include "score/midinote.h"
#include "score/lyricscontext.h"
#include "score/syllable.h"
#include "score/mark.h"
#include "score/articulation.h"
#include "score/barline.h"

#include "score/functionmarkcontext.h"
#include "score/functionmark.h"

#include "score/chordnamecontext.h"
#include "score/chordname.h"

#include "interface/mididevice.h" // needed for midiPitch->diatonicPitch

#define INITIAL_X_OFFSET 20 // space between the left border and the first music element
#define MINIMUM_SPACE 10    // minimum space between the music elements

QList<CADrawableMusElement*> CALayoutEngine::scalableElts;
int *CALayoutEngine::streamsRehersalMarks;

/*!
	\class CAEngraver
	\brief Class for correctly placing the abstract notes to the score canvas.

	This class is a bridge between the data part of Canorus and the UI.
	Out of data CAMusElement* and CAContext* objects, it creates their CADrawable* instances.
*/

/*!
	Repositions the notes in the abstract sheet of the given score view \a v so they fit nicely.
	This function doesn't clear the view, but only adds the elements.
*/
void CALayoutEngine::reposit( CAScoreView *v ) {
	//int i;
	CASheet *sheet = v->sheet();

	//list of all the music element lists (ie. streams) taken from all the contexts
	QList< QList<CAMusElement*> > musStreamList; // streams music elements
	QList<CAContext*> contexts; // which context does the stream belong to

	int dy = 50;
	QList<int> nonFirstVoiceIdxs;	//list of indexes of musStreamLists which the voices aren't the first voice. This is used later for determining should a sign be created or not (if it has been created in 1st voice already, don't recreate it in the other voices in the same staff).
	QMap<CAContext*, CADrawableContext*> drawableContextMap;

	for (int i=0; i < sheet->contextList().size(); i++) {
        switch (sheet->contextList()[i]->contextType()) {
            case CAContext::Staff: {
                if (i > 0) dy += 70;

                CAStaff *staff = static_cast<CAStaff *>(sheet->contextList()[i]);
                drawableContextMap[staff] = new CADrawableStaff(staff, 0, dy);
                v->addCElement(drawableContextMap[staff]);

                //add all the voices lists to the common list
                for (int j = 0; j < staff->voiceList().size(); j++) {
                    musStreamList << staff->voiceList()[j]->musElementList();
                    contexts << staff;
                    if (staff->voiceList()[j]->voiceNumber() != 1)
                        nonFirstVoiceIdxs << musStreamList.size() - 1;
                }
                dy += drawableContextMap[staff]->height();
                break;
            }
            case CAContext::LyricsContext: {
                CALyricsContext *lyricsContext = static_cast<CALyricsContext *>(sheet->contextList()[i]);
                if (i > 0 && (sheet->contextList()[i - 1]->contextType() != CAContext::LyricsContext ||
                              static_cast<CALyricsContext *>(sheet->contextList()[i - 1])->associatedVoice()->staff() !=
                              lyricsContext->associatedVoice()->staff())) {
                    dy += 70; // the previous context wasn't lyrics or was not related to the current lyrics
                }

                drawableContextMap[lyricsContext] = new CADrawableLyricsContext(lyricsContext, 0, dy);
                v->addCElement(drawableContextMap[lyricsContext]);

                // convert QList<CASyllable*> to QList<CAMusElement*>
                QList<CAMusElement *> syllableList;
                for (int i = 0; i < lyricsContext->syllableList().size(); i++) {
                    syllableList << lyricsContext->syllableList()[i];
                }

                musStreamList << syllableList;
                contexts << lyricsContext;
                dy += drawableContextMap[lyricsContext]->height();
                break;
            }
            case CAContext::FiguredBassContext: {
                if (i > 0) dy += 70;

                CAFiguredBassContext *fbContext = static_cast<CAFiguredBassContext *>(sheet->contextList()[i]);
                drawableContextMap[fbContext] = new CADrawableFiguredBassContext(fbContext, 0, dy);
                v->addCElement(drawableContextMap[fbContext]);
                QList<CAFiguredBassMark *> fbmList = fbContext->figuredBassMarkList();
                // TODO: Is there a faster way to cast QList<CAFiguredBassMark*> to QList<CAMusElement*>?
                QList<CAMusElement *> musList;
                for (int j = 0; j < fbmList.size(); j++) musList << fbmList[j];
                musStreamList << musList;
                contexts << fbContext;
                dy += drawableContextMap[fbContext]->height();
                break;
            }
            case CAContext::FunctionMarkContext: {
                if (i > 0 && sheet->contextList()[i - 1]->contextType() != CAContext::FiguredBassContext) {
                    dy += 70;
                }

                CAFunctionMarkContext *fmContext = static_cast<CAFunctionMarkContext *>(sheet->contextList()[i]);
                drawableContextMap[fmContext] = new CADrawableFunctionMarkContext(fmContext, 0, dy);
                v->addCElement(drawableContextMap[fmContext]);
                QList<CAFunctionMark *> fmList = fmContext->functionMarkList();
                // TODO: Is there a faster way to cast QList<CAFunctionMark*> to QList<CAMusElement*>?
                QList<CAMusElement *> musList;
                for (int j = 0; j < fmList.size(); j++) musList << fmList[j];
                musStreamList << musList;
                contexts << fmContext;
                dy += drawableContextMap[fmContext]->height();
                break;
            }
            case CAContext::ChordNameContext: {
                if (i > 0) dy += 70;

                CAChordNameContext *cnContext = static_cast<CAChordNameContext *>(sheet->contextList()[i]);
                drawableContextMap[cnContext] = new CADrawableChordNameContext(cnContext, 0, dy);
                v->addCElement(drawableContextMap[cnContext]);
                QList<CAChordName *> cnList = cnContext->chordNameList();
                // TODO: Is there a faster way to cast QList<CAChordName*> to QList<CAMusElement*>?
                QList<CAMusElement *> musList;
                for (int j = 0; j < cnList.size(); j++) musList << cnList[j];
                musStreamList << musList;
                contexts << cnContext;
                dy += drawableContextMap[cnContext]->height();
                break;
            }
        }
	}

	int streams = musStreamList.size();
	int *streamsIdx = new int[streams];
	for (int i=0; i<streams; i++) streamsIdx[i] = 0;
	int *streamsX = new int[streams];
	for (int i=0; i<streams; i++) streamsX[i] = INITIAL_X_OFFSET;
    CALayoutEngine::streamsRehersalMarks = new int[streams];
	for (int i=0; i<streams; i++) streamsRehersalMarks[i] = 0;
	CAClef **lastClef = new CAClef *[streams];
	for (int i=0; i<streams; i++) lastClef[i] = nullptr;
	CAKeySignature **lastKeySig = new CAKeySignature *[streams];
	for (int i=0; i<streams; i++) lastKeySig[i] = nullptr;
	CATimeSignature **lastTimeSig = new CATimeSignature *[streams];
	for (int i=0; i<streams; i++) lastTimeSig[i] = nullptr;
	scalableElts.clear();

	int timeStart = 0;
	bool done = false;
	CADrawableFunctionMarkSupport **lastDFMTonicizations = new CADrawableFunctionMarkSupport *[streams];
	for (int i=0; i<streams; i++) lastDFMTonicizations[i] = nullptr;
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
		//bool placedSymbol = false;	//used if waiting for notes to gather and a non-time-consuming symbol has been placed
		for (int i=0; i < streams; i++) {
			//loop until the first playable element
			//multiple elements can have the same start time. eg. Clef + Key signature + Time signature + First note
			while ( (streamsIdx[i] < musStreamList[i].size()) &&
			        ((elt = musStreamList[i].at(streamsIdx[i]))->timeStart() == timeStart) &&
			        (!elt->isPlayable()) &&
					(elt->musElementType() != CAMusElement::Barline) &&	        //barlines should be aligned
			        (elt->musElementType() != CAMusElement::FunctionMark) &&	//function marks are placed separately
			        (elt->musElementType() != CAMusElement::FiguredBassMark) &&	//figured bass marks are placed separately
					(elt->musElementType() != CAMusElement::Syllable) &&	    //syllables are placed separately
					(elt->musElementType() != CAMusElement::ChordName)	        //chord names are placed separately
				  ) {
				drawableContext = drawableContextMap[elt->context()];

				//place signs in first voices
				if ( (drawableContext->drawableContextType() == CADrawableContext::DrawableStaff) &&
				     (!nonFirstVoiceIdxs.contains(i)) ) {
					switch ( elt->musElementType() ) {
						case CAMusElement::Clef: {
							CADrawableClef *clef = new CADrawableClef(
								static_cast<CAClef*>(elt),
								static_cast<CADrawableStaff*>(drawableContext),
								streamsX[i],
								drawableContext->yPos()
							);

							v->addMElement(clef);

							// set the last clefs in all voices in the same staff
							for (int j=0; j<contexts.size(); j++)
								if (contexts[j]==contexts[i])
									lastClef[j] = clef->clef();

							streamsX[i] += (clef->neededWidth() + MINIMUM_SPACE);
							//placedSymbol = true;

							placeMarks( clef, v, i );

							break;
						}
						case CAMusElement::KeySignature: {
							CADrawableKeySignature *keySig = new CADrawableKeySignature(
								static_cast<CAKeySignature*>(elt),
								static_cast<CADrawableStaff*>(drawableContext),
								streamsX[i],
								drawableContext->yPos()
							);

							v->addMElement(keySig);

							// set the last key sigs in all voices in the same staff
							for (int j=0; j<contexts.size(); j++)
								if (contexts[j]==contexts[i])
									lastKeySig[j] = keySig->keySignature();

							streamsX[i] += (keySig->neededWidth() + MINIMUM_SPACE);
							//placedSymbol = true;

							placeMarks( keySig, v, i );

							break;
						}
						case CAMusElement::TimeSignature: {
							CADrawableTimeSignature *timeSig = new CADrawableTimeSignature(
								static_cast<CATimeSignature*>(elt),
								static_cast<CADrawableStaff*>(drawableContext),
								streamsX[i],
								drawableContext->yPos()
							);

							v->addMElement(timeSig);

							// set the last time signatures in all voices in the same staff
							for (int j=0; j<contexts.size(); j++)
								if (contexts[j]==contexts[i])
									lastTimeSig[j] = timeSig->timeSignature();

							streamsX[i] += (timeSig->neededWidth() + MINIMUM_SPACE);
							//placedSymbol = true;

							placeMarks( timeSig, v, i );

							break;
						}
						default:
							qDebug() << "Warning: CALayoutEngine::reposit - Unhandled Element" << elt->musElementType();
							break;
					} // SWITCH

				} // IF firstVoice
				streamsIdx[i]++;
			}
		}

		// Draw function key name, if needed
		QList<CADrawableFunctionMarkSupport*> lastDFMKeyNames;
		for (int i=0;
		     (i<streams) &&
		     (streamsIdx[i] < musStreamList[i].size()) &&
			 (musStreamList[i].at(streamsIdx[i])->timeStart() == timeStart);
			 i++) {
			CAMusElement *elt = musStreamList[i].at(streamsIdx[i]);
			if (elt->musElementType()==CAMusElement::FunctionMark && (static_cast<CAFunctionMark*>(elt))->function()!=CAFunctionMark::Undefined) {
				drawableContext = drawableContextMap[elt->context()];
				if (streamsIdx[i]-1<0 ||
					static_cast<CAFunctionMark*>(musStreamList[i].at(streamsIdx[i]-1))->key() != static_cast<CAFunctionMark*>(elt)->key()
				   ) {
					//draw new function mark key, if it was changed or if it's the first function in the score
					CADrawableFunctionMarkSupport *support = new CADrawableFunctionMarkSupport(
						CADrawableFunctionMarkSupport::Key,
						CADiatonicKey::diatonicKeyToString(static_cast<CAFunctionMark*>(elt)->key()),
						drawableContext,
						streamsX[i],
						static_cast<CADrawableFunctionMarkContext*>(drawableContext)->yPosLine(CADrawableFunctionMarkContext::Middle)
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
					static_cast<CABarline*>(elt),
					static_cast<CADrawableStaff*>(drawableContext),
					streamsX[i],
					drawableContext->yPos()
				);

				v->addMElement(bar);
				//placedSymbol = true;
				streamsX[i] += (bar->neededWidth() + MINIMUM_SPACE);
				streamsIdx[i] = streamsIdx[i] + 1;

				placeMarks( bar, v, i );
				placeNoteCheckerErrors( bar, v );
			}
		}

		// Synchronize minimum X-es between the contexts - all the noteheads or barlines should be horizontally aligned.
		for (int i=0; i<streams; i++) maxX = (streamsX[i] > maxX) ? streamsX[i] : maxX;
		for (int i=0; i<streams; i++)
			if (musStreamList[i].size() &&
				musStreamList[i].last()->musElementType()!=CAMusElement::FunctionMark)
				streamsX[i] = maxX;

		// Place accidentals and key names of the function marks, if needed.
		// These elements are so called Support elements. They can't be selected and they're not really connected usually to any logical element, but they're needed when drawing.
		double maxWidth = 0;
		double maxAccidentalXEnd = 0;
		QList<CADrawableAccidental*> lastAccidentals;
		for (int i=0; i < streams; i++) {
			// loop until the element has come, which has bigger timeStart
			CADrawableMusElement *newElt = nullptr;
			int oldStreamIdx = streamsIdx[i];
			while ( (streamsIdx[i] < musStreamList[i].size()) &&
			        ((elt = musStreamList[i].at(streamsIdx[i]))->timeStart() == timeStart) &&
			        (elt->isPlayable())
			      ) {
				drawableContext = drawableContextMap[elt->context()];

				if (elt->musElementType()==CAMusElement::Note &&
					static_cast<CADrawableStaff*>(drawableContext)->getAccs(streamsX[i], static_cast<CANote*>(elt)->diatonicPitch().noteName()) != static_cast<CANote*>(elt)->diatonicPitch().accs()
				   ) {
						newElt = new CADrawableAccidental(
							static_cast<signed char>(static_cast<CANote*>(elt)->diatonicPitch().accs()),
							static_cast<CANote*>(elt),
							static_cast<CADrawableStaff*>(drawableContext),
							streamsX[i],
							static_cast<CADrawableStaff*>(drawableContext)->calculateCenterYCoord(static_cast<CANote*>(elt), lastClef[i])
						);

						v->addMElement(newElt);
						lastAccidentals << static_cast<CADrawableAccidental*>(newElt);
						if (newElt->neededWidth() > maxWidth)
							maxWidth = newElt->neededWidth();
						if (maxAccidentalXEnd < newElt->neededWidth()+newElt->xPos())
							maxAccidentalXEnd = newElt->neededWidth()+newElt->xPos();
				}

				streamsIdx[i]++;
			}
			streamsIdx[i] = oldStreamIdx;

			streamsX[i] += ((maxWidth!=0.0)?(maxWidth+1):0);	// append the needed space for the last used note
		}

		// Synchronize minimum X-es between the contexts - all the noteheads or barlines should be horizontally aligned.
		for (int i=0; i<streams; i++) maxX = (streamsX[i] > maxX) ? streamsX[i] : maxX;
		for (int i=0; i<streams; i++) streamsX[i] = maxX;

		// Align support elements (accidentals, function key names) to the right
		for (int i=0; i<lastDFMKeyNames.size(); i++)
			lastDFMKeyNames[i]->setXPos(maxX - lastDFMKeyNames[i]->neededWidth() - 2);

		double deltaXPos = maxX - maxAccidentalXEnd;
		for (int i=0; i<lastAccidentals.size(); i++) {
			lastAccidentals[i]->setXPos(lastAccidentals[i]->xPos()+deltaXPos-1);
		}

		// Place noteheads and other elements aligned to noteheads (syllables, function marks)
		for (int i=0; i < streams; i++) {
			// loop until the element has come, which has bigger timeStart
			while ( (streamsIdx[i] < musStreamList[i].size()) &&
			        ((elt = musStreamList[i].at(streamsIdx[i]))->timeStart() == timeStart) &&
			        (elt->isPlayable() ||
			         elt->musElementType()==CAMusElement::FiguredBassMark ||
			         elt->musElementType()==CAMusElement::FunctionMark ||
			         elt->musElementType()==CAMusElement::Syllable ||
			         elt->musElementType()==CAMusElement::ChordName
			        )
			      ) {
				drawableContext = drawableContextMap[elt->context()];
				CADrawableMusElement *newElt=nullptr;

				switch ( elt->musElementType() ) {
					case CAMusElement::Note: {
						newElt = new CADrawableNote(
							static_cast<CANote*>(elt),
							drawableContext,
							streamsX[i],
							static_cast<CADrawableStaff*>(drawableContext)->calculateCenterYCoord(static_cast<CANote*>(elt), lastClef[i])
						);

						// Create Ties
						if ( static_cast<CADrawableNote*>(newElt)->note()->tieStart() ) {
							CASlur::CASlurDirection dir = static_cast<CADrawableNote*>(newElt)->note()->tieStart()->slurDirection();
							if ( dir==CASlur::SlurPreferred || dir==CASlur::SlurNeutral )
								dir = static_cast<CADrawableNote*>(newElt)->note()->actualSlurDirection();
							CADrawableSlur *tie=nullptr;
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
							CADrawableSlur *slur=nullptr;
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
							if( dSlur )
							{
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
						}

						// Create Phrasing Slurs
						if ( static_cast<CADrawableNote*>(newElt)->note()->phrasingSlurStart() ) {
							CASlur::CASlurDirection dir = static_cast<CADrawableNote*>(newElt)->note()->phrasingSlurStart()->slurDirection();
							if ( dir==CASlur::SlurPreferred || dir==CASlur::SlurNeutral)
								dir = static_cast<CADrawableNote*>(newElt)->note()->actualSlurDirection();
							CADrawableSlur *phrasingSlur=nullptr;
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

						// add tuplet - same as for the rests
						if ( static_cast<CADrawableNote*>(newElt)->note()->isLastInTuplet() ) {
							double x1 = v->findMElement(static_cast<CADrawableNote*>(newElt)->note()->tuplet()->firstNote())->xPos();
							double x2 = newElt->xPos() + newElt->width();
							double y1 = v->findMElement(static_cast<CADrawableNote*>(newElt)->note()->tuplet()->firstNote())->yPos();
							if ( y1 > drawableContext->yPos() && y1 < drawableContext->yPos()+drawableContext->height() ) {
								y1 = drawableContext->yPos()+drawableContext->height() + 10; // inside the staff
							} else if ( y1 < drawableContext->yPos() ){
								y1 -= 10; // above the staff
							} else {
								y1 += 10; // under the staff
							}
							double y2 = v->findMElement(static_cast<CADrawableNote*>(newElt)->note()->tuplet()->lastNote())->yPos();
							if ( y2 > drawableContext->yPos() && y2 < drawableContext->yPos()+drawableContext->height() ) {
								y2 = drawableContext->yPos()+drawableContext->height() + 10; // inside the staff
							} else if ( y2 < drawableContext->yPos() ){
								y2 -= 10; // above the staff
							} else {
								y2 += 10; // under the staff
							}

							CADrawableTuplet *dTuplet = new CADrawableTuplet( static_cast<CADrawableNote*>(newElt)->note()->tuplet(), drawableContext, x1, y1, x2, y2 );
							v->addMElement(dTuplet);
						}

						if ( static_cast<CANote*>(elt)->isLastInChord() )
							streamsX[i] += (newElt->neededWidth() + MINIMUM_SPACE);

						placeMarks( newElt, v, i );

						break;
					}
					case CAMusElement::Rest: {
						newElt = new CADrawableRest(
							static_cast<CARest*>(elt),
							drawableContext,
							streamsX[i],
							drawableContext->yPos()
						);

						v->addMElement(newElt);
						streamsX[i] += (newElt->neededWidth() + MINIMUM_SPACE);

						// add tuplet - same as for the notes
						if ( static_cast<CADrawableRest*>(newElt)->rest()->isLastInTuplet() ) {
							double x1 = v->findMElement(static_cast<CADrawableRest*>(newElt)->rest()->tuplet()->firstNote())->xPos();
							double x2 = newElt->xPos() + newElt->width();
							double y1 = v->findMElement(static_cast<CADrawableRest*>(newElt)->rest()->tuplet()->firstNote())->yPos();
							if ( y1 > drawableContext->yPos() && y1 < drawableContext->yPos()+drawableContext->height() ) {
								y1 = drawableContext->yPos()+drawableContext->height() + 10; // inside the staff
							} else if ( y1 < drawableContext->yPos() ){
								y1 -= 10; // above the staff
							} else {
								y1 += 10; // under the staff
							}
							double y2 = v->findMElement(static_cast<CADrawableRest*>(newElt)->rest()->tuplet()->lastNote())->yPos();
							if ( y2 > drawableContext->yPos() && y2 < drawableContext->yPos()+drawableContext->height() ) {
								y2 = drawableContext->yPos()+drawableContext->height() + 10; // inside the staff
							} else if ( y2 < drawableContext->yPos() ){
								y2 -= 10; // above the staff
							} else {
								y2 += 10; // under the staff
							}

							CADrawableTuplet *dTuplet = new CADrawableTuplet( static_cast<CADrawableRest*>(newElt)->rest()->tuplet(), drawableContext, x1, y1, x2, y2 );
							v->addMElement(dTuplet);
						}

						placeMarks( newElt, v, i );

						break;
					}
					case CAMusElement::MidiNote: {
						CADrawableStaff* dStaff = static_cast<CADrawableStaff*>(drawableContext);
						CAMidiNote *midiNote = static_cast<CAMidiNote*>(elt);
						int pitch = 0;
						if (lastKeySig[i]) {
							pitch = CADiatonicPitch::diatonicPitchFromMidiPitchKey( midiNote->midiPitch(), lastKeySig[i]->diatonicKey() ).noteName();
						} else {
							pitch = CADiatonicPitch::diatonicPitchFromMidiPitch( midiNote->midiPitch() ).noteName();
						}
						newElt = new CADrawableMidiNote( midiNote, dStaff, streamsX[i], dStaff->calculateCenterYCoord( pitch, streamsX[i] ) );

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
						CADrawableMusElement *prevDSyllable = (prevSyllable?v->findMElement(prevSyllable):nullptr);
						if (prevDSyllable) {
							prevDSyllable->setWidth( newElt->xPos() - prevDSyllable->xPos() );
						}

						v->addMElement(newElt);
						streamsX[i] += (newElt->neededWidth() + MINIMUM_SPACE);
						break;
					}
					case CAMusElement::FiguredBassMark: {
						CAFiguredBassMark *fbm = static_cast<CAFiguredBassMark*>(elt);
						for (int j=fbm->numbers().size()-1; j>=0; j--) {
							newElt = new CADrawableFiguredBassNumber(
								fbm,
								fbm->numbers()[j],
								static_cast<CADrawableFiguredBassContext*>(drawableContext),
								// shift figured bass mark more right, if only one character
								streamsX[i]+((!fbm->accs().contains(fbm->numbers()[j]) || fbm->numbers()[j]==0)?3:0),
								drawableContext->yPos()+CADrawableFiguredBassNumber::DEFAULT_NUMBER_SIZE*(fbm->numbers().size()-1-j)
							);

							v->addMElement(newElt);
						}

						streamsX[i] += (newElt?newElt->neededWidth():0 + MINIMUM_SPACE);
						break;
					}
					case CAMusElement::FunctionMark: {
						CAFunctionMark *function = static_cast<CAFunctionMark*>(elt);

						// Make a new line, if parallel function present
						if (streamsIdx[i]-1>=0 && musStreamList[i].at(streamsIdx[i]-1)->timeStart()==musStreamList[i].at(streamsIdx[i])->timeStart()) {
							static_cast<CADrawableFunctionMarkContext*>(drawableContext)->nextLine();
							CADrawableFunctionMarkSupport *newKey = new CADrawableFunctionMarkSupport(
								CADrawableFunctionMarkSupport::Key,
								CADiatonicKey::diatonicKeyToString(function->key()),
								drawableContext,
								streamsX[i],
								static_cast<CADrawableFunctionMarkContext*>(drawableContext)->yPosLine(CADrawableFunctionMarkContext::Middle)
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
						CADrawableFunctionMarkSupport *alterations=nullptr;
						if (function->addedDegrees().size() || function->alteredDegrees().size()) {
							alterations = new CADrawableFunctionMarkSupport(
								CADrawableFunctionMarkSupport::Alterations,
								function,
								drawableContext,
								streamsX[i],
								static_cast<CADrawableFunctionMarkContext*>(drawableContext)->yPosLine(CADrawableFunctionMarkContext::Lower)+3
							);
							// center-align alterations to function, if placed
							if (newElt)
								alterations->setXPos(newElt->xPos()+newElt->width()/2.0-alterations->width()/2.0+0.5);
							else	//center-align to note
								alterations->setXPos(streamsX[i]+5-alterations->width()/2.0+0.5);
						}

						// Place tonicization. The same tonicization is always placed from streamsIdx[i]-nth to streamsIdx[i]-1th element, where streamsIdx[i] is the current index.
						int j=streamsIdx[i]-1;	//index of the previous elt
						CADrawableFunctionMarkSupport *tonicization=nullptr;
						for (; j>=0 && static_cast<CAFunctionMark*>(musStreamList[i].at(j))->function()==CAFunctionMark::Undefined; j--);	//ignore any alterations back there
						if (j>=0 && (
						    // place tonicization, if tonic degree is not default
							static_cast<CAFunctionMark*>(musStreamList[i].at(j))->tonicDegree()!=CAFunctionMark::T &&
							static_cast<CAFunctionMark*>(musStreamList[i].at(j))->tonicDegree()!=CAFunctionMark::Undefined
							// and it's not still the same
							&& (function->tonicDegree()!=static_cast<CAFunctionMark*>(musStreamList[i].at(j))->tonicDegree() || function->key()!=static_cast<CAFunctionMark*>(musStreamList[i].at(j))->key())
						    // always place tonicization, if ellipse is present
						    //|| ((CAFunctionMark*)musStreamList[i].at(j))->isPartOfEllipse()
						)) {
							//CAFunctionMark::CAFunctionType type = ((CAFunctionMark*)musStreamList[i].at(j))->tonicDegree();
							CAFunctionMark *right = static_cast<CAFunctionMark*>(musStreamList[i].at(j));

							// find the n-th element back
							while (--j>=0 && static_cast<CAFunctionMark*>(musStreamList[i].at(j))->tonicDegree()==static_cast<CAFunctionMark*>(musStreamList[i].at(j+1))->tonicDegree() &&
								   static_cast<CAFunctionMark*>(musStreamList[i].at(j))->key()==static_cast<CAFunctionMark*>(musStreamList[i].at(j+1))->key());
							CAFunctionMark *tonicStart = static_cast<CAFunctionMark*>(musStreamList[i].at(++j));
							CADrawableFunctionMark *left = static_cast<CADrawableFunctionMark*>(drawableContext->findMElement(tonicStart));
							if (tonicStart!=static_cast<CAMusElement*>(musStreamList[i].at(streamsIdx[i]-1))) {	// tonicization isn't single (more than 1 tonic element)
								tonicization = new CADrawableFunctionMarkSupport(
									CADrawableFunctionMarkSupport::Tonicization,
									left,
									static_cast<CADrawableFunctionMarkContext*>(drawableContext),
									left->xPos(),
									static_cast<CADrawableFunctionMarkContext*>(drawableContext)->yPosLine(CADrawableFunctionMarkContext::Middle),
									static_cast<CADrawableFunctionMark*>(drawableContext->findMElement(right))
								);
							} else {																// tonicization is single (one tonic)
								tonicization = new CADrawableFunctionMarkSupport(
									CADrawableFunctionMarkSupport::Tonicization,
									left,
									static_cast<CADrawableFunctionMarkContext*>(drawableContext),
									left->xPos(),
									static_cast<CADrawableFunctionMarkContext*>(drawableContext)->yPosLine(CADrawableFunctionMarkContext::Middle)
								);
								tonicization->setXPos(left->xPos()+0.5*left->width()-0.5*tonicization->width()+0.5);	// align center
							}
						}

						// Place horizontal modulation rectangle, if needed
						j=streamsIdx[i]-1;
						CADrawableFunctionMarkSupport *hModulationRect=nullptr;
						if (newElt && j>=0) {
							if (static_cast<CAFunctionMark*>(musStreamList[i].at(j))->key()!=function->key() &&
								static_cast<CAFunctionMark*>(musStreamList[i].at(j))->timeStart()!=function->timeStart()) {
								CADrawableFunctionMark *left = static_cast<CADrawableFunctionMark*>(drawableContext->findMElement(musStreamList[i].at(j)));
								hModulationRect = new CADrawableFunctionMarkSupport(
									CADrawableFunctionMarkSupport::Rectangle,
									left,
									static_cast<CADrawableFunctionMarkContext*>(drawableContext),
									left->xPos(),
									left->yPos(),
									static_cast<CADrawableFunctionMark*>(newElt)
								);
								if (streamsIdx[i]%2)
									hModulationRect->setRectWider(true);	//make it wider, so it potentially doesn't overlap with other rectangles around
						    }
						}

						// Place vertical modulation rectangle, if needed
						// This must be done *before* the extender lines are placed!
						j=streamsIdx[i]-1;
						CADrawableFunctionMarkSupport *vModulationRect=nullptr;
						while (--j>=0 &&
							   static_cast<CAFunctionMark*>(musStreamList[i].at(j))->key()!=static_cast<CAFunctionMark*>(musStreamList[i].at(j+1))->key() &&
							   static_cast<CAFunctionMark*>(musStreamList[i].at(j))->timeStart()==static_cast<CAFunctionMark*>(musStreamList[i].at(j+1))->timeStart()
						      );
						if (++j>=0 && j!=streamsIdx[i]-1) {
							CADrawableFunctionMark *left = static_cast<CADrawableFunctionMark*>(drawableContext->findMElement(musStreamList[i].at(j)));
							vModulationRect = new CADrawableFunctionMarkSupport(
								CADrawableFunctionMarkSupport::Rectangle,
								left,
								static_cast<CADrawableFunctionMarkContext*>(drawableContext),
								left->xPos(),
								left->yPos(),
								static_cast<CADrawableFunctionMark*>(drawableContext->findMElement(musStreamList[i].at(streamsIdx[i]-1)))
							);
						}

						// Place horizontal chord area rectangle for the previous elements, if element neighbours are of the same chordarea/function
						j=streamsIdx[i]-1;
						CADrawableFunctionMarkSupport *hChordAreaRect=nullptr;
						if (j>=0 && // don't draw rectangle, if the current element would still be in the rectangle
						    (
							 (static_cast<CAFunctionMark*>(musStreamList[i].at(j))->key()==function->key() &&
							  static_cast<CAFunctionMark*>(musStreamList[i].at(j))->function()!=function->function() &&
							  static_cast<CAFunctionMark*>(musStreamList[i].at(j))->function()!=function->chordArea() &&
							  static_cast<CAFunctionMark*>(musStreamList[i].at(j))->chordArea()!=function->chordArea())
							 || static_cast<CAFunctionMark*>(musStreamList[i].at(j))->key()!=function->key()
						     || j==musStreamList[i].size()
						    )
						   ) {
							bool oneFunctionOnly=true;
							while (--j>=0 &&
								   static_cast<CAFunctionMark*>(musStreamList[i].at(j+1))->chordArea()!=CAFunctionMark::Undefined &&
								   static_cast<CAFunctionMark*>(musStreamList[i].at(j))->key()==static_cast<CAFunctionMark*>(musStreamList[i].at(j+1))->key() &&
									(static_cast<CAFunctionMark*>(musStreamList[i].at(j))->chordArea()==static_cast<CAFunctionMark*>(musStreamList[i].at(j+1))->chordArea() ||
									 static_cast<CAFunctionMark*>(musStreamList[i].at(j))->function()==static_cast<CAFunctionMark*>(musStreamList[i].at(j+1))->chordArea() ||
									 static_cast<CAFunctionMark*>(musStreamList[i].at(j))->chordArea()==static_cast<CAFunctionMark*>(musStreamList[i].at(j+1))->function()
							        )
							      )
								if ( static_cast<CAFunctionMark*>(musStreamList[i].at(j))->function()!=static_cast<CAFunctionMark*>(musStreamList[i].at(j+1))->function() )
									oneFunctionOnly = false;

							if ( ++j != streamsIdx[i]-1 && !oneFunctionOnly ) {
								CADrawableFunctionMark *left = static_cast<CADrawableFunctionMark*>(drawableContext->findMElement(musStreamList[i].at(j)));
								hChordAreaRect = new CADrawableFunctionMarkSupport(
									CADrawableFunctionMarkSupport::Rectangle,
									left,
									static_cast<CADrawableFunctionMarkContext*>(drawableContext),
									left->xPos(),
									left->yPos(),
									static_cast<CADrawableFunctionMark*>(drawableContext->findMElement(musStreamList[i].at(streamsIdx[i]-1)))
								);
							}
						}

						// Place chordarea mark below in paranthesis, if no neighbours are of same chordarea
						CADrawableFunctionMarkSupport *chordArea=nullptr;
						j=streamsIdx[i];
						if (newElt &&
						    function->chordArea()!=CAFunctionMark::Undefined &&
						    function->chordArea()!=function->function() && // chord area is the same as function name - don't draw chordarea then
						    (j-1<0 ||
							 ((static_cast<CAFunctionMark*>(musStreamList[i].at(j-1))->key()==function->key() &&
							  static_cast<CAFunctionMark*>(musStreamList[i].at(j-1))->chordArea()!=function->chordArea() &&
							  static_cast<CAFunctionMark*>(musStreamList[i].at(j-1))->function()!=function->chordArea() &&
							  static_cast<CAFunctionMark*>(musStreamList[i].at(j-1))->chordArea()!=function->function()) ||
							  static_cast<CAFunctionMark*>(musStreamList[i].at(j-1))->tonicDegree()!=function->tonicDegree() ||
							  static_cast<CAFunctionMark*>(musStreamList[i].at(j-1))->key()!=function->key()
						     )
						    ) &&
						    (j+1>=musStreamList[i].size() ||
							 ((static_cast<CAFunctionMark*>(musStreamList[i].at(j+1))->key()==function->key() &&
							  static_cast<CAFunctionMark*>(musStreamList[i].at(j+1))->chordArea()!=function->chordArea() &&
							  static_cast<CAFunctionMark*>(musStreamList[i].at(j+1))->function()!=function->chordArea() &&
							  static_cast<CAFunctionMark*>(musStreamList[i].at(j+1))->chordArea()!=function->function()) ||
							  static_cast<CAFunctionMark*>(musStreamList[i].at(j+1))->tonicDegree()!=function->tonicDegree() ||
							  static_cast<CAFunctionMark*>(musStreamList[i].at(j+1))->key()!=function->key()
						     )
						    )
						   ) {
							chordArea = new CADrawableFunctionMarkSupport(
								CADrawableFunctionMarkSupport::ChordArea,
								static_cast<CADrawableFunctionMark*>(newElt),
								static_cast<CADrawableFunctionMarkContext*>(drawableContext),
								streamsX[i],
								static_cast<CADrawableFunctionMarkContext*>(drawableContext)->yPosLine(CADrawableFunctionMarkContext::Lower)
							);
							chordArea->setXPos(newElt->xPos()-(chordArea->width()-newElt->width())/2.0 + 0.5);
						}

						// Place ellipse
						j=streamsIdx[i]-1;
						CADrawableFunctionMarkSupport *ellipse=nullptr;
						if (j>=0 && static_cast<CAFunctionMark*>(musStreamList[i].at(j))->isPartOfEllipse()	//place ellipse, if it has it
							&& (!function->isPartOfEllipse()) ) {	//and it's not lasting anymore
							//find the n-th element back
							while (--j>=0 && static_cast<CAFunctionMark*>(musStreamList[i].at(j))->isPartOfEllipse());
							CAFunctionMark *ellipseStart = static_cast<CAFunctionMark*>(musStreamList[i].at(++j));
							CADrawableFunctionMark *left = static_cast<CADrawableFunctionMark*>(drawableContext->findMElement(ellipseStart));
							ellipse = new CADrawableFunctionMarkSupport(
								CADrawableFunctionMarkSupport::Ellipse,
								left,
								static_cast<CADrawableFunctionMarkContext*>(drawableContext),
								left->xPos(),
								static_cast<CADrawableFunctionMarkContext*>(drawableContext)->yPosLine(CADrawableFunctionMarkContext::Lower),
								static_cast<CADrawableFunctionMark*>(drawableContext->findMElement(static_cast<CAMusElement*>(musStreamList[i].at(streamsIdx[i]-1))))
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
										tonicization->setWidth( lastDFMKeyNames.last()->xPos()-tonicization->xPos() );
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
									prevDFM->setWidth( lastDFMKeyNames.last()->xPos()-prevDFM->xPos() );
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
                    case CAMusElement::ChordName: {
                        newElt = new CADrawableChordName(
                                static_cast<CAChordName*>(elt),
                                static_cast<CADrawableChordNameContext*>(drawableContext),
                                streamsX[i],
                                drawableContext->yPos() + qRound(CADrawableChordNameContext::DEFAULT_CHORDNAME_VERTICAL_SPACING)
                        );

                        CAMusElement *prevChordName = drawableContext->context()->previous(elt);
						CADrawableMusElement *prevDChordName = (prevChordName?v->findMElement(prevChordName):nullptr);
                        if (prevDChordName) {
                            prevDChordName->setWidth( newElt->xPos() - prevDChordName->xPos() );
                        }

                        v->addMElement(newElt);

						placeNoteCheckerErrors( newElt, v );

						streamsX[i] += (newElt->neededWidth() + MINIMUM_SPACE);
                        break;
                    }

					default:
						qDebug() << "Warning: CALayoutEngine::reposit2 - Unhandled Element" << elt->musElementType();
						break;
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
	delete [] streamsIdx;
	delete [] streamsX;
	delete [] streamsRehersalMarks;
	delete [] lastClef;
	delete [] lastKeySig;
	delete [] lastTimeSig;
	delete [] lastDFMTonicizations;
}

/*!
	Place marks for the given music element.
*/
void CALayoutEngine::placeMarks( CADrawableMusElement *e, CAScoreView *v, int streamIdx ) {
	CAMusElement *elt = e->musElement();
    double xCoord = e->xPos();

    for ( int i=0,j=0,k=0; i < elt->markList().size(); i++ ) { // j/k are vertical offsets above/below the staff
		if ( elt->markList()[i]->isCommon() &&
		     elt->musElementType()==CAMusElement::Note &&
		     !static_cast<CANote*>(elt)->isFirstInChord() ) {
			continue;
		}

		CAMark *mark = elt->markList()[i];

        double yCoord;
		CAFingering *fingering = dynamic_cast<CAFingering*>(mark);
		if ( mark->markType()==CAMark::Pedal ||
			 (fingering && (fingering->fingerList()[0]==CAFingering::LHeel || fingering->fingerList()[0]==CAFingering::LToe)) ||
			 (elt->musElementType()==CAMusElement::Note && static_cast<CANote*>(elt)->actualSlurDirection()==CASlur::SlurDown &&
			   ((mark->markType()==CAMark::Text) || (mark->markType()==CAMark::Fermata) || (mark->markType()==CAMark::Articulation) ))) {
			// place mark below
			xCoord = e->xPos();
			yCoord = qMax(e->yPos()+e->height(),e->drawableContext()->yPos()+e->drawableContext()->height())+20*(k+1);
			k++;
		} else if ( elt->musElementType()==CAMusElement::Note &&
					static_cast<CANote*>(elt)->isPartOfChord() &&
					fingering && fingering->fingerList()[0] < 6 ) {
			// place mark beside the note
			xCoord = e->xPos() + e->width();
			yCoord = e->yPos() - 2;
        } else if ( mark->markType()==CAMark::Articulation &&
                    static_cast<CAArticulation*>(mark)->articulationType()==CAArticulation::Breath ) {
            // place breath mark above right
            xCoord = e->xPos() + 1.2*e->width();
            yCoord = qMin(e->yPos()-2.5*e->height(), e->drawableContext()->yPos()-20);
        } else if ( mark->markType()==CAMark::Articulation ) {
            // place other articulation marks above center
            xCoord = e->xPos() + e->width()/2.0 - 3;
            yCoord = qMin(e->yPos(),e->drawableContext()->yPos())-20*(j+1);
            j++;
        } else {
			// place mark above
			xCoord = e->xPos();
			yCoord = qMin(e->yPos(),e->drawableContext()->yPos())-20*(j+1);
			j++;
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

void CALayoutEngine::placeNoteCheckerErrors( CADrawableMusElement* dMusElt, CAScoreView* v ) {
	QList<CANoteCheckerError*> ncErrors = dMusElt->musElement()->noteCheckerErrorList();
	for (int i=0; i<ncErrors.size(); i++) {
		v->addDrawableNoteCheckerError(
			new CADrawableNoteCheckerError(ncErrors[i], dMusElt)
		);
	}
}
