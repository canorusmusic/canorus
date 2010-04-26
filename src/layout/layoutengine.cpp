/*!
	Copyright (c) 2006-2010, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QList>
#include <iostream>	//debug
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

#include "layout/drawablelyricscontext.h"
#include "layout/drawablesyllable.h"

#include "layout/drawablefiguredbasscontext.h"
#include "layout/drawablefiguredbassnumber.h"

#include "layout/drawablefunctionmarkcontext.h"
#include "layout/drawablefunctionmark.h"

#include "score/sheet.h"

#include "score/staff.h"
#include "score/voice.h"
#include "score/lyricscontext.h"
#include "score/functionmarkcontext.h"
#include "score/functionmark.h"

#include "score/note.h"
#include "score/rest.h"
#include "score/keysignature.h"
#include "score/timesignature.h"
#include "score/barline.h"
#include "score/midinote.h"
#include "score/syllable.h"

#include "interface/mididevice.h" // needed for midiPitch->diatonicPitch

const double CALayoutEngine::INITIAL_X_OFFSET = 20;
const double CALayoutEngine::RIGHT_X_OFFSET = 100;
const double CALayoutEngine::MINIMUM_SPACE = 10;

/*!
	\class CALayoutEngine
	\brief Class for correctly placing the abstract notes to the score canvas.

	This class is a bridge between the data part of Canorus and the UI.
	Out of data CAMusElement* and CAContext* objects, it creates their CADrawable* instances.
*/

/*!
	Default constructor.
 */
CALayoutEngine::CALayoutEngine( CAScoreView *v )
 : _scoreView(v) {
}

/*!
	Repositions the notes in the abstract sheet of the given score view \a v so they fit nicely.
	This function doesn't clear the view, but only adds the elements.
*/
void CALayoutEngine::reposit() {
	int i;
	CASheet *sheet = _scoreView->sheet();

	//list of all the music element lists (ie. streams) taken from all the contexts
	QList< QList<CAMusElement*> > musStreamList; // streams music elements
	QList<CAContext*> contexts; // which context does the stream belong to

	double dy = 50;
	QList<int> nonFirstVoiceIdxs;	//list of indexes of musStreamLists which the voices aren't the first voice. This is used later for determining should a sign be created or not (if it has been created in 1st voice already, don't recreate it in the other voices in the same staff).
	_drawableContextMap.clear();

	for (int i=0; i < sheet->contextList().size(); i++) {
		if (sheet->contextList()[i]->contextType() == CAContext::Staff) {
			if (i>0) dy+=70;

			CAStaff *staff = static_cast<CAStaff*>(sheet->contextList()[i]);
			_drawableContextMap[staff] = new CADrawableStaff(staff);
			_scoreView->addCElement(_drawableContextMap[staff]);
			_drawableContextMap[staff]->setPos( 0, dy );

			//add all the voices lists to the common list
			for (int j=0; j < staff->voiceList().size(); j++) {
				musStreamList << staff->voiceList()[j]->musElementList();
				contexts << staff;
				if (staff->voiceList()[j]->voiceNumber()!=1)
					nonFirstVoiceIdxs << musStreamList.size()-1;
			}
			dy += _drawableContextMap[staff]->boundingRect().height();
		} else
		if (sheet->contextList()[i]->contextType() == CAContext::LyricsContext) {
			CALyricsContext *lyricsContext = static_cast<CALyricsContext*>(sheet->contextList()[i]);
			if (i>0 && (sheet->contextList()[i-1]->contextType() != CAContext::LyricsContext ||
			    static_cast<CALyricsContext*>(sheet->contextList()[i-1])->associatedVoice()->staff()!=lyricsContext->associatedVoice()->staff())) {
				dy+=70; // the previous context wasn't lyrics or was not related to the current lyrics
			}

			_drawableContextMap[lyricsContext] = new CADrawableLyricsContext(lyricsContext);
			_scoreView->addCElement(_drawableContextMap[lyricsContext]);
			_drawableContextMap[lyricsContext]->setPos(0, dy);

			// convert QList<CASyllable*> to QList<CAMusElement*>
			QList<CAMusElement*> syllableList;
			for (int i=0; i<lyricsContext->syllableList().size(); i++) {
				syllableList << lyricsContext->syllableList()[i];
			}

			musStreamList << syllableList;
			contexts << lyricsContext;
			dy += _drawableContextMap[lyricsContext]->boundingRect().height();
		} else
		if (sheet->contextList()[i]->contextType() == CAContext::FiguredBassContext) {
			if (i>0) dy+=70;

			CAFiguredBassContext *fbContext = static_cast<CAFiguredBassContext*>(sheet->contextList()[i]);
			_drawableContextMap[fbContext] = new CADrawableFiguredBassContext(fbContext);
			_scoreView->addCElement(_drawableContextMap[fbContext]);
			_drawableContextMap[fbContext]->setPos(0, dy);
			QList<CAFiguredBassMark*> fbmList = fbContext->figuredBassMarkList();
			QList<CAMusElement*> musList; for (int i=0; i<fbmList.size(); i++) musList << fbmList[i];
			musStreamList << musList;
			contexts << fbContext;
			dy += _drawableContextMap[fbContext]->boundingRect().height();
		} else
		if (sheet->contextList()[i]->contextType() == CAContext::FunctionMarkContext) {
			if (i>0 && sheet->contextList()[i-1]->contextType() != CAContext::FiguredBassContext) {
				dy+=70;
			}

			CAFunctionMarkContext *fmContext = static_cast<CAFunctionMarkContext*>(sheet->contextList()[i]);
			_drawableContextMap[fmContext] = new CADrawableFunctionMarkContext(fmContext);
			_scoreView->addCElement(_drawableContextMap[fmContext]);
			_drawableContextMap[fmContext]->setPos(0, dy);
			QList<CAFunctionMark*> fmList = fmContext->functionMarkList();
			QList<CAMusElement*> musList; for (int i=0; i<fmList.size(); i++) musList << fmList[i];
			musStreamList << musList;
			contexts << fmContext;
			dy += _drawableContextMap[fmContext]->boundingRect().height();
		}
	}

	int streams = musStreamList.size();
	int streamsIdx[streams]; for (int i=0; i<streams; i++) streamsIdx[i] = 0;
	int streamsX[streams]; for (int i=0; i<streams; i++) streamsX[i] = INITIAL_X_OFFSET;
	int streamsRehersalMarks[streams]; for (int i=0; i<streams; i++) streamsRehersalMarks[i] = 0;
	_streamsRehersalMarks = streamsRehersalMarks;
	CAClef *lastClef[streams]; for (int i=0; i<streams; i++) lastClef[i] = 0;
	CAKeySignature *lastKeySig[streams]; for (int i=0; i<streams; i++) lastKeySig[i] = 0;
	CATimeSignature *lastTimeSig[streams]; for (int i=0; i<streams; i++) lastTimeSig[i] = 0;
	_scalableElts.clear();

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
			        (elt->musElementType() != CAMusElement::FiguredBassMark) &&	//figured bass marks are placed separately
			        (elt->musElementType() != CAMusElement::Syllable)	//syllables are placed separately
			      ) {
				drawableContext = _drawableContextMap[elt->context()];

				//place signs in first voices
				if ( (drawableContext->drawableContextType() == CADrawableContext::DrawableStaff) &&
				     (!nonFirstVoiceIdxs.contains(i)) ) {
					switch ( elt->musElementType() ) {
						case CAMusElement::Clef: {
							CADrawableClef *clef = new CADrawableClef(
								static_cast<CAClef*>(elt),
								static_cast<CADrawableStaff*>(drawableContext)
							);

							_scoreView->addMElement(clef);
							clef->setPos( streamsX[i], drawableContext->pos().y() );

							// set the last clefs in all voices in the same staff
							for (int j=0; j<contexts.size(); j++)
								if (contexts[j]==contexts[i])
									lastClef[j] = clef->clef();

							streamsX[i] += (clef->boundingRect().width() + MINIMUM_SPACE);
							placedSymbol = true;

							placeMarks( clef, i );

							break;
						}
						case CAMusElement::KeySignature: {
							CADrawableKeySignature *keySig = new CADrawableKeySignature(
								static_cast<CAKeySignature*>(elt),
								static_cast<CADrawableStaff*>(drawableContext),
								lastClef[i],
								lastKeySig[i]
							);

							_scoreView->addMElement(keySig);
							keySig->setPos( streamsX[i], drawableContext->pos().y() );

							// set the last key sigs in all voices in the same staff
							for (int j=0; j<contexts.size(); j++)
								if (contexts[j]==contexts[i])
									lastKeySig[j] = keySig->keySignature();

							streamsX[i] += (keySig->boundingRect().width() + MINIMUM_SPACE);
							placedSymbol = true;

							placeMarks( keySig, i );

							break;
						}
						case CAMusElement::TimeSignature: {
							CADrawableTimeSignature *timeSig = new CADrawableTimeSignature(
								static_cast<CATimeSignature*>(elt),
								static_cast<CADrawableStaff*>(drawableContext)
							);

							_scoreView->addMElement(timeSig);
							timeSig->setPos( streamsX[i], drawableContext->pos().y() );

							// set the last time signatures in all voices in the same staff
							for (int j=0; j<contexts.size(); j++)
								if (contexts[j]==contexts[i])
									lastTimeSig[j] = timeSig->timeSignature();

							streamsX[i] += (timeSig->boundingRect().width() + MINIMUM_SPACE);
							placedSymbol = true;

							placeMarks( timeSig, i );

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
			 (musStreamList[i].at(streamsIdx[i])->timeStart() == timeStart);
			 i++) {
			CAMusElement *elt = musStreamList[i].at(streamsIdx[i]);
			if (elt->musElementType()==CAMusElement::FunctionMark && ((CAFunctionMark*)elt)->function()!=CAFunctionMark::Undefined) {
				drawableContext = _drawableContextMap[elt->context()];
				if (streamsIdx[i]-1<0 ||
				    ((CAFunctionMark*)musStreamList[i].at(streamsIdx[i]-1))->key() != ((CAFunctionMark*)elt)->key()
				   ) {
					//draw new function mark key, if it was changed or if it's the first function in the score
					CADrawableFunctionMarkSupport *support = new CADrawableFunctionMarkSupport(
						CADrawableFunctionMarkSupport::Key,
						CADiatonicKey::diatonicKeyToString(static_cast<CAFunctionMark*>(elt)->key()),
						drawableContext
					);
					streamsX[i] += (support->boundingRect().width());
					_scoreView->addMElement(support);
					support->setPos( streamsX[i], static_cast<CADrawableFunctionMarkContext*>(drawableContext)->yPosLine(CADrawableFunctionMarkContext::Middle) );
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

				drawableContext = _drawableContextMap[elt->context()];
				CADrawableBarline *bar = new CADrawableBarline(
					static_cast<CABarline*>(elt),
					static_cast<CADrawableStaff*>(drawableContext)
				);

				_scoreView->addMElement(bar);
				bar->setPos( streamsX[i], drawableContext->pos().y() );
				placedSymbol = true;
				streamsX[i] += (bar->boundingRect().width() + MINIMUM_SPACE);
				streamsIdx[i] = streamsIdx[i] + 1;

				placeMarks( bar, i );
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
				drawableContext = _drawableContextMap[elt->context()];

				if (elt->musElementType()==CAMusElement::Note &&
				    ((CADrawableStaff*)drawableContext)->getAccs(streamsX[i], static_cast<CANote*>(elt)->diatonicPitch().noteName()) != static_cast<CANote*>(elt)->diatonicPitch().accs()
				   ) {
						QList<CANote*> chord = static_cast<CANote*>(elt)->getChord();
						newElt = new CADrawableAccidental(
							static_cast<CANote*>(elt)->diatonicPitch().accs(),
							static_cast<CANote*>(elt),
							static_cast<CADrawableStaff*>(drawableContext)
						);

						_scoreView->addMElement(newElt);

						// shift every second accidental to the left, to avoid clashing in chord
						double x = streamsX[i];
						int idx = chord.indexOf(static_cast<CANote*>(elt));
						if (idx>0 && idx%2 && ((CADrawableStaff*)drawableContext)->getAccs(streamsX[i], chord[idx-1]->diatonicPitch().noteName()) != chord[idx-1]->diatonicPitch().accs()) {
							x -= 9.0 + (idx%4?1.5:-1.0);
						}
						newElt->setPos( x, drawableContext->pos().y()+static_cast<CADrawableStaff*>(drawableContext)->calculateCenterYCoord(static_cast<CANote*>(elt), lastClef[i]) );

						lastAccidentals << static_cast<CADrawableAccidental*>(newElt);
						if (newElt->boundingRect().width() > maxWidth)
							maxWidth = newElt->boundingRect().width();
						if (maxAccidentalXEnd < newElt->boundingRect().width()+newElt->pos().x())
							maxAccidentalXEnd = newElt->boundingRect().width()+newElt->pos().x();
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
/*		for (int i=0; i<lastDFMKeyNames.size(); i++)
			lastDFMKeyNames[i]->setXPos(maxX - lastDFMKeyNames[i]->boundingRect().width() - 2);
*/
		int deltaXPos = maxX - maxAccidentalXEnd;
/*		for (int i=0; i<lastAccidentals.size(); i++) {
			lastAccidentals[i]->setXPos(lastAccidentals[i]->pos().x()+deltaXPos-1);
		}
*/
		// Place noteheads and other elements aligned to noteheads (syllables, function marks)
		for (int i=0; i < streams; i++) {
			// loop until the element has come, which has bigger timeStart
			while ( (streamsIdx[i] < musStreamList[i].size()) &&
			        ((elt = musStreamList[i].at(streamsIdx[i]))->timeStart() == timeStart) &&
			        (elt->isPlayable() ||
			         elt->musElementType()==CAMusElement::FiguredBassMark ||
			         elt->musElementType()==CAMusElement::FunctionMark ||
			         elt->musElementType()==CAMusElement::Syllable
			        )
			      ) {
				drawableContext = _drawableContextMap[elt->context()];
				CADrawableMusElement *newElt=0;

				switch ( elt->musElementType() ) {
					case CAMusElement::Note: {
						newElt = new CADrawableNote(
							static_cast<CANote*>(elt),
							drawableContext
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
									newElt->pos().x()+newElt->boundingRect().width(), newElt->pos().y(),
									newElt->pos().x() + 20, newElt->pos().y() - 5,
									newElt->pos().x() + 40, newElt->pos().y()
								);
							} else
							if (dir==CASlur::SlurDown) {
								tie = new CADrawableSlur(
									static_cast<CADrawableNote*>(newElt)->note()->tieStart(), drawableContext,
									newElt->pos().x()+newElt->boundingRect().width(), newElt->pos().y() + newElt->boundingRect().height(),
									newElt->pos().x() + 20, newElt->pos().y() + newElt->boundingRect().height() + 5,
									newElt->pos().x() + 40, newElt->pos().y()	+ newElt->boundingRect().height()
								);
							}
							_scoreView->addMElement(tie);
							tie->setPos(newElt->pos().x()+newElt->boundingRect().width(), newElt->pos().y());
						}
						if ( static_cast<CADrawableNote*>(newElt)->note()->tieEnd() ) {
							// Set the slur coordinates for the second note
							CASlur::CASlurDirection dir = static_cast<CADrawableNote*>(newElt)->note()->tieEnd()->slurDirection();
							if ( dir==CASlur::SlurPreferred || dir==CASlur::SlurNeutral )
								dir = static_cast<CADrawableNote*>(newElt)->note()->tieEnd()->noteStart()->actualSlurDirection();
							CADrawableSlur *dSlur = static_cast<CADrawableSlur*>(_scoreView->findMElement(static_cast<CADrawableNote*>(newElt)->note()->tieEnd()));
							dSlur->setX2( newElt->pos().x() );
							dSlur->setXMid( qRound(0.5*dSlur->pos().x() + 0.5*newElt->pos().x()) );
							if ( dir==CASlur::SlurUp ) {
								dSlur->setY2( newElt->pos().y() );
								dSlur->setYMid( qMin( dSlur->y2(), dSlur->y1() ) - 5 );
							} else
							if ( dir==CASlur::SlurDown ) {
								dSlur->setY2( newElt->pos().y() + newElt->boundingRect().height() );
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
									newElt->pos().x()+newElt->boundingRect().width(), newElt->pos().y(),
									newElt->pos().x() + 20, newElt->pos().y() - 15,
									newElt->pos().x() + 40, newElt->pos().y()
								);
							} else
							if (dir==CASlur::SlurDown) {
								slur = new CADrawableSlur(
									static_cast<CADrawableNote*>(newElt)->note()->slurStart(), drawableContext,
									newElt->pos().x()+newElt->boundingRect().width(), newElt->pos().y() + newElt->boundingRect().height(),
									newElt->pos().x() + 20, newElt->pos().y() + newElt->boundingRect().height() + 15,
									newElt->pos().x() + 40, newElt->pos().y()	+ newElt->boundingRect().height()
								);
							}
							_scoreView->addMElement(slur);
							slur->setPos(newElt->pos().x()+newElt->boundingRect().width(), newElt->pos().y());
						}
						if ( static_cast<CADrawableNote*>(newElt)->note()->slurEnd() ) {
							// Set the slur coordinates for the second note
							CASlur::CASlurDirection dir = static_cast<CADrawableNote*>(newElt)->note()->slurEnd()->slurDirection();
							if ( dir==CASlur::SlurPreferred || dir==CASlur::SlurNeutral )
								dir = static_cast<CADrawableNote*>(newElt)->note()->slurEnd()->noteStart()->actualSlurDirection();
							CADrawableSlur *dSlur = static_cast<CADrawableSlur*>(_scoreView->findMElement(static_cast<CADrawableNote*>(newElt)->note()->slurEnd()));
							dSlur->setX2( newElt->pos().x() );
							dSlur->setXMid( qRound(0.5*dSlur->pos().x() + 0.5*newElt->pos().x()) );
							if ( dir==CASlur::SlurUp ) {
								dSlur->setY2( newElt->pos().y() );
								dSlur->setYMid( qMin( dSlur->y2(), dSlur->y1() ) - 15 );
							} else
							if ( dir==CASlur::SlurDown ) {
								dSlur->setY2( newElt->pos().y() + newElt->boundingRect().height() );
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
									newElt->pos().x()+newElt->boundingRect().width(), newElt->pos().y(),
									newElt->pos().x() + 20, newElt->pos().y() - 19,
									newElt->pos().x() + 40, newElt->pos().y()
								);
							} else
							if (dir==CASlur::SlurDown) {
								phrasingSlur = new CADrawableSlur(
									static_cast<CADrawableNote*>(newElt)->note()->phrasingSlurStart(), drawableContext,
									newElt->pos().x()+newElt->boundingRect().width(), newElt->pos().y() + newElt->boundingRect().height(),
									newElt->pos().x() + 20, newElt->pos().y() + newElt->boundingRect().height() + 19,
									newElt->pos().x() + 40, newElt->pos().y()	+ newElt->boundingRect().height()
								);
							}
							_scoreView->addMElement(phrasingSlur);
							phrasingSlur->setPos(newElt->pos().x()+newElt->boundingRect().width(), newElt->pos().y());
						}
						if ( static_cast<CADrawableNote*>(newElt)->note()->phrasingSlurEnd() ) {
							// Set the slur coordinates for the second note
							CASlur::CASlurDirection dir = static_cast<CADrawableNote*>(newElt)->note()->phrasingSlurEnd()->slurDirection();
							if ( dir==CASlur::SlurPreferred || dir==CASlur::SlurNeutral )
								dir = static_cast<CADrawableNote*>(newElt)->note()->phrasingSlurEnd()->noteStart()->actualSlurDirection();
							CADrawableSlur *dSlur = static_cast<CADrawableSlur*>(_scoreView->findMElement(static_cast<CADrawableNote*>(newElt)->note()->phrasingSlurEnd()));
							dSlur->setX2( newElt->pos().x() );
							dSlur->setXMid( qRound(0.5*dSlur->pos().x() + 0.5*newElt->pos().x()) );
							if ( dir==CASlur::SlurUp ) {
								dSlur->setY2( newElt->pos().y() );
								dSlur->setYMid( qMin( dSlur->y2(), dSlur->y1() ) - 19 );
							} else
							if ( dir==CASlur::SlurDown ) {
								dSlur->setY2( newElt->pos().y() + newElt->boundingRect().height() );
								dSlur->setYMid( qMax( dSlur->y2(), dSlur->y1() ) + 19 );
							}
						}

						_scoreView->addMElement(newElt);
						newElt->setPos( streamsX[i], drawableContext->pos().y()+static_cast<CADrawableStaff*>(drawableContext)->calculateCenterYCoord(static_cast<CANote*>(elt), lastClef[i]) );

						// add tuplet - same as for the rests
						if ( static_cast<CADrawableNote*>(newElt)->note()->isLastInTuplet() ) {
							int x1 = _scoreView->findMElement(static_cast<CADrawableNote*>(newElt)->note()->tuplet()->firstNote())->pos().x();
							int x2 = newElt->pos().x() + newElt->boundingRect().width();
							int y1 = _scoreView->findMElement(static_cast<CADrawableNote*>(newElt)->note()->tuplet()->firstNote())->pos().y();
							if ( y1 > drawableContext->pos().y() && y1 < drawableContext->pos().y()+drawableContext->boundingRect().height() ) {
								y1 = drawableContext->pos().y()+drawableContext->boundingRect().height() + 10; // inside the staff
							} else if ( y1 < drawableContext->pos().y() ){
								y1 -= 10; // above the staff
							} else {
								y1 += 10; // under the staff
							}
							int y2 = _scoreView->findMElement(static_cast<CADrawableNote*>(newElt)->note()->tuplet()->lastNote())->pos().y();
							if ( y2 > drawableContext->pos().y() && y2 < drawableContext->pos().y()+drawableContext->boundingRect().height() ) {
								y2 = drawableContext->pos().y()+drawableContext->boundingRect().height() + 10; // inside the staff
							} else if ( y2 < drawableContext->pos().y() ){
								y2 -= 10; // above the staff
							} else {
								y2 += 10; // under the staff
							}

							CADrawableTuplet *dTuplet = new CADrawableTuplet( static_cast<CADrawableNote*>(newElt)->note()->tuplet(), drawableContext, x1, y1, x2, y2 );
							_scoreView->addMElement(dTuplet);
							dTuplet->setPos( x1, y1 );
						}

						if ( static_cast<CANote*>(elt)->isLastInChord() )
							streamsX[i] += (newElt->boundingRect().width() + MINIMUM_SPACE);

						placeMarks( newElt, i );

						break;
					}
					case CAMusElement::Rest: {
						newElt = new CADrawableRest(
							static_cast<CARest*>(elt),
							drawableContext
						);

						_scoreView->addMElement(newElt);
						newElt->setPos( streamsX[i], drawableContext->pos().y() );
						streamsX[i] += (newElt->boundingRect().width() + MINIMUM_SPACE);

						// add tuplet - same as for the notes
						if ( static_cast<CADrawableRest*>(newElt)->rest()->isLastInTuplet() ) {
							int x1 = _scoreView->findMElement(static_cast<CADrawableRest*>(newElt)->rest()->tuplet()->firstNote())->pos().x();
							int x2 = newElt->pos().x() + newElt->boundingRect().width();
							int y1 = _scoreView->findMElement(static_cast<CADrawableRest*>(newElt)->rest()->tuplet()->firstNote())->pos().y();
							if ( y1 > drawableContext->pos().y() && y1 < drawableContext->pos().y()+drawableContext->boundingRect().height() ) {
								y1 = drawableContext->pos().y()+drawableContext->boundingRect().height() + 10; // inside the staff
							} else if ( y1 < drawableContext->pos().y() ){
								y1 -= 10; // above the staff
							} else {
								y1 += 10; // under the staff
							}
							int y2 = _scoreView->findMElement(static_cast<CADrawableRest*>(newElt)->rest()->tuplet()->lastNote())->pos().y();
							if ( y2 > drawableContext->pos().y() && y2 < drawableContext->pos().y()+drawableContext->boundingRect().height() ) {
								y2 = drawableContext->pos().y()+drawableContext->boundingRect().height() + 10; // inside the staff
							} else if ( y2 < drawableContext->pos().y() ){
								y2 -= 10; // above the staff
							} else {
								y2 += 10; // under the staff
							}

							CADrawableTuplet *dTuplet = new CADrawableTuplet( static_cast<CADrawableRest*>(newElt)->rest()->tuplet(), drawableContext, x1, y1, x2, y2 );
							_scoreView->addMElement(dTuplet);
						}

						placeMarks( newElt, i );

						break;
					}
					case CAMusElement::MidiNote: {
						CADrawableStaff* dStaff = static_cast<CADrawableStaff*>(drawableContext);
						CAMidiNote *midiNote = static_cast<CAMidiNote*>(elt);
						int pitch = CAMidiDevice::midiPitchToDiatonicPitch( midiNote->midiPitch() ).noteName();

						newElt = new CADrawableMidiNote( midiNote, dStaff );
						_scoreView->addMElement(newElt);
						newElt->setPos( streamsX[i], dStaff->pos().y() + dStaff->calculateCenterYCoord( pitch, streamsX[i] ) );
						break;
					}
					case CAMusElement::Syllable: {
						newElt = new CADrawableSyllable(
							static_cast<CASyllable*>(elt),
							static_cast<CADrawableLyricsContext*>(drawableContext)
						);

						CAMusElement *prevSyllable = drawableContext->context()->previous(elt);
						CADrawableMusElement *prevDSyllable = (prevSyllable?_scoreView->findMElement(prevSyllable):0);
						if (prevDSyllable) {
/*							prevDSyllable->setWidth( newElt->pos().x() - prevDSyllable->pos().x() );*/
						}

						_scoreView->addMElement(newElt);
						newElt->setPos( streamsX[i], drawableContext->pos().y() + qRound(CADrawableLyricsContext::DEFAULT_TEXT_VERTICAL_SPACING) );

						streamsX[i] += (newElt->boundingRect().width() + MINIMUM_SPACE);
						break;
					}
					case CAMusElement::FiguredBassMark: {
						CAFiguredBassMark *fbm = static_cast<CAFiguredBassMark*>(elt);
						for (int j=fbm->numbers().size()-1; j>=0; j--) {
							newElt = new CADrawableFiguredBassNumber(
								fbm,
								fbm->numbers()[j],
								static_cast<CADrawableFiguredBassContext*>(drawableContext)
							);

							_scoreView->addMElement(newElt);
							newElt->setPos(
								// shift figured bass mark more right, if only one character
								streamsX[i]+((!fbm->accs().contains(fbm->numbers()[j]) || fbm->numbers()[j]==0)?3:0),
								drawableContext->pos().y()+CADrawableFiguredBassNumber::DEFAULT_NUMBER_SIZE*(fbm->numbers().size()-1-j)
							);
						}

						streamsX[i] += (newElt?newElt->boundingRect().width():0 + MINIMUM_SPACE);
						break;
					}
					case CAMusElement::FunctionMark: {
						CAFunctionMark *function = static_cast<CAFunctionMark*>(elt);
						CADrawableFunctionMark *left = 0;

						// Make a new line, if parallel function present
						if (streamsIdx[i]-1>=0 && musStreamList[i].at(streamsIdx[i]-1)->timeStart()==musStreamList[i].at(streamsIdx[i])->timeStart()) {
							((CADrawableFunctionMarkContext*)drawableContext)->nextLine();
							CADrawableFunctionMarkSupport *newKey = new CADrawableFunctionMarkSupport(
								CADrawableFunctionMarkSupport::Key,
								CADiatonicKey::diatonicKeyToString(function->key()),
								drawableContext
							);
/*							newKey->setXPos(streamsX[i]-newKey->boundingRect().width()-2);
*/							_scoreView->addMElement(newKey);
							newKey->setPos(streamsX[i], static_cast<CADrawableFunctionMarkContext*>(drawableContext)->yPosLine(CADrawableFunctionMarkContext::Middle));
						}

						// Place the function itself, if it's independent
						newElt = new CADrawableFunctionMark(
							function,
							static_cast<CADrawableFunctionMarkContext*>(drawableContext)
						);

						// Place alterations
						CADrawableFunctionMarkSupport *alterations=0;
						if (function->addedDegrees().size() || function->alteredDegrees().size()) {
							alterations = new CADrawableFunctionMarkSupport(
								CADrawableFunctionMarkSupport::Alterations,
								function,
								drawableContext
							);
							// center-align alterations to function, if placed
/*							if (newElt)
								alterations->setXPos((int)(newElt->pos().x()+newElt->boundingRect().width()/2.0-alterations->width()/2.0+0.5));
							else	//center-align to note
								alterations->setXPos((int)(streamsX[i]+5-alterations->boundingRect().width()/2.0+0.5));
*/						}

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
							left = static_cast<CADrawableFunctionMark*>(drawableContext->findMElement(tonicStart));
							if (tonicStart!=(CAMusElement*)musStreamList[i].at(streamsIdx[i]-1)) {	// tonicization isn't single (more than 1 tonic element)
								tonicization = new CADrawableFunctionMarkSupport(
									CADrawableFunctionMarkSupport::Tonicization,
									left,
									static_cast<CADrawableFunctionMarkContext*>(drawableContext),
									static_cast<CADrawableFunctionMark*>(drawableContext->findMElement(right))
								);
							} else {																// tonicization is single (one tonic)
								tonicization = new CADrawableFunctionMarkSupport(
									CADrawableFunctionMarkSupport::Tonicization,
									left,
									(CADrawableFunctionMarkContext*)drawableContext
								);
/*								tonicization->setXPos((int)(left->pos().x()+0.5*left->boundingRect().width()-0.5*tonicization->width()+0.5));	// align center
*/							}
						}

						// Place horizontal modulation rectangle, if needed
						j=streamsIdx[i]-1;
						CADrawableFunctionMarkSupport *hModulationRect=0;
						if (newElt && j>=0) {
							if (((CAFunctionMark*)musStreamList[i].at(j))->key()!=function->key() &&
							    ((CAFunctionMark*)musStreamList[i].at(j))->timeStart()!=function->timeStart()) {
								left = static_cast<CADrawableFunctionMark*>(drawableContext->findMElement(musStreamList[i].at(j)));
								hModulationRect = new CADrawableFunctionMarkSupport(
									CADrawableFunctionMarkSupport::Rectangle,
									left,
									static_cast<CADrawableFunctionMarkContext*>(drawableContext),
									static_cast<CADrawableFunctionMark*>(newElt)
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
							left = static_cast<CADrawableFunctionMark*>(drawableContext->findMElement(musStreamList[i].at(j)));
							vModulationRect = new CADrawableFunctionMarkSupport(
								CADrawableFunctionMarkSupport::Rectangle,
								left,
								static_cast<CADrawableFunctionMarkContext*>(drawableContext),
								static_cast<CADrawableFunctionMark*>(drawableContext->findMElement(musStreamList[i].at(streamsIdx[i]-1)))
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
								left = static_cast<CADrawableFunctionMark*>(drawableContext->findMElement(musStreamList[i].at(j)));
								hChordAreaRect = new CADrawableFunctionMarkSupport(
									CADrawableFunctionMarkSupport::Rectangle,
									left,
									static_cast<CADrawableFunctionMarkContext*>(drawableContext),
									static_cast<CADrawableFunctionMark*>(drawableContext->findMElement(musStreamList[i].at(streamsIdx[i]-1)))
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
								static_cast<CADrawableFunctionMark*>(newElt),
								static_cast<CADrawableFunctionMarkContext*>(drawableContext)
							);
/*							chordArea->setXPos((int)(newElt->pos().x()-(chordArea->boundingRect().width()-newElt->width())/2.0 + 0.5));
*/						}

						// Place ellipse
						j=streamsIdx[i]-1;
						CADrawableFunctionMarkSupport *ellipse=0;
						if (j>=0 && ((CAFunctionMark*)musStreamList[i].at(j))->isPartOfEllipse()	//place ellipse, if it has it
							&& (!function->isPartOfEllipse()) ) {	//and it's not lasting anymore
							//find the n-th element back
							while (--j>=0 && ((CAFunctionMark*)musStreamList[i].at(j))->isPartOfEllipse());
							CAFunctionMark *ellipseStart = (CAFunctionMark*)musStreamList[i].at(++j);
							left = static_cast<CADrawableFunctionMark*>(drawableContext->findMElement(ellipseStart));
							ellipse = new CADrawableFunctionMarkSupport(
								CADrawableFunctionMarkSupport::Ellipse,
								left,
								static_cast<CADrawableFunctionMarkContext*>(drawableContext),
								static_cast<CADrawableFunctionMark*>(drawableContext->findMElement((CAMusElement*)musStreamList[i].at(streamsIdx[i]-1)))
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
					/*				if ( prevElt->key()==function->key() )
										tonicization->setWidth( newElt->pos().x()-tonicization->pos().x() );
									else
										tonicization->setWidth( lastDFMKeyNames.last()->pos().x()-tonicization->pos().x() );
					*/			}

								if (lastDFMTonicizations[i]) {
									//lastDFMTonicizations[i]->setExtenderLineVisible(true);
					/*				if ( prevElt->key()==function->key() )
										lastDFMTonicizations[i]->setWidth( newElt->pos().x()-lastDFMTonicizations[i]->pos().x() );
									else
										lastDFMTonicizations[i]->setWidth( lastDFMKeyNames[i]->pos().x()-lastDFMTonicizations[i]->pos().x() );
					*/			}
							}

							if ( prevDFM && prevDFM->isExtenderLineVisible() ) {
/*								if ( prevElt->key()==function->key() )
									prevDFM->setWidth( newElt->pos().x()-prevDFM->pos().x() );
								else
									prevDFM->setWidth( lastDFMKeyNames.last()->pos().x()-prevDFM->pos().x() );
*/							}
						}

						if (newElt) {
							_scoreView->addMElement(newElt);	// when only alterations are made and no function placed, IF is needed
							newElt->setPos(streamsX[i],
							(function->tonicDegree()==CAFunctionMark::T && (!function->isPartOfEllipse()))?
								static_cast<CADrawableFunctionMarkContext*>(drawableContext)->yPosLine(CADrawableFunctionMarkContext::Middle):
								static_cast<CADrawableFunctionMarkContext*>(drawableContext)->yPosLine(CADrawableFunctionMarkContext::Upper));
						}
						if (tonicization) {
							_scoreView->addMElement(tonicization);
							tonicization->setPos( left->pos().x(), static_cast<CADrawableFunctionMarkContext*>(drawableContext)->yPosLine(CADrawableFunctionMarkContext::Middle) );
							lastDFMTonicizations[i]=tonicization;
						}
						if (ellipse) {
							_scoreView->addMElement(ellipse);
							ellipse->setPos( left->pos().x(), static_cast<CADrawableFunctionMarkContext*>(drawableContext)->yPosLine(CADrawableFunctionMarkContext::Lower) );
						}
						if (hModulationRect) {
							_scoreView->addMElement(hModulationRect);
							hModulationRect->setPos( left->pos().x(), left->pos().y() );
						}
						if (vModulationRect) {
							_scoreView->addMElement(vModulationRect);
							vModulationRect->setPos( left->pos().x(), left->pos().y() );
						}
						if (hChordAreaRect) {
							_scoreView->addMElement(hChordAreaRect);
							hChordAreaRect->setPos( left->pos().x(), left->pos().y() );
						}
						if (chordArea) {
							_scoreView->addMElement(chordArea);
							chordArea->setPos( streamsX[i], static_cast<CADrawableFunctionMarkContext*>(drawableContext)->yPosLine(CADrawableFunctionMarkContext::Lower) );
						}
						if (alterations) {
							_scoreView->addMElement(alterations);
							alterations->setPos( streamsX[i], static_cast<CADrawableFunctionMarkContext*>(drawableContext)->yPosLine(CADrawableFunctionMarkContext::Lower)+3 );
						}

						if (newElt && streamsIdx[i]+1<musStreamList[i].size() && musStreamList[i].at(streamsIdx[i]+1)->timeStart()!=musStreamList[i].at(streamsIdx[i])->timeStart()) {
							streamsX[i] += (newElt->boundingRect().width());
						}

						break;
					}
				}

				streamsIdx[i]++;
			}

		}
	}

	// reposit the scalable elements (eg. crescendo)
	for (int i=0; i<_scalableElts.size(); i++) {
/*		_scalableElts[i]->setXPos( _scoreView->timeToCoords(_scalableElts[i]->musElement()->timeStart()) );
		_scalableElts[i]->setWidth( _scoreView->timeToCoords(_scalableElts[i]->musElement()->timeEnd()) - _scalableElts[i]->pos().x() );
*/		_scoreView->addMElement(_scalableElts[i]);
	}

	// set correct width for the contexts, to fit all the containing elements
	updateContextsWidth();
}

/*!
	Place marks for the given music element.
*/
void CALayoutEngine::placeMarks( CADrawableMusElement *e, int streamIdx ) {
	CAMusElement *elt = e->musElement();
	int xCoord = e->pos().x();

	for ( int i=0,j=0,k=0; i < elt->markList().size(); i++ ) {
		if ( elt->markList()[i]->isCommon() &&
		     elt->musElementType()==CAMusElement::Note &&
		     !static_cast<CANote*>(elt)->isFirstInChord() ) {
			continue;
		}

		CAMark *mark = elt->markList()[i];

		int yCoord;
		if ( mark->markType()==CAMark::Pedal ||
		     ( mark->markType()==CAMark::Fingering && (static_cast<CAFingering*>(mark)->fingerList()[0]==CAFingering::LHeel || static_cast<CAFingering*>(mark)->fingerList()[0]==CAFingering::LToe )) ||
		     elt->musElementType()==CAMusElement::Note && static_cast<CANote*>(elt)->actualSlurDirection()==CASlur::SlurDown &&
		       (mark->markType()==CAMark::Text || mark->markType()==CAMark::Fermata || mark->markType()==CAMark::Articulation || mark->markType()==CAMark::Fingering && static_cast<CAFingering*>(mark)->fingerList()[0]!=CAFingering::LHeel && static_cast<CAFingering*>(mark)->fingerList()[0]!=CAFingering::LToe ))  {
			// place mark below
			xCoord = e->pos().x();
			yCoord = qMax(e->pos().y()+e->boundingRect().height(),e->drawableContext()->pos().y()+e->drawableContext()->boundingRect().height())+20*(k+1);
			k++;
		} else if ( elt->musElementType()==CAMusElement::Note &&
				    static_cast<CANote*>(elt)->isPartOfChord() &&
		            mark->markType()==CAMark::Fingering && static_cast<CAFingering*>(mark)->fingerList()[0] < 6 ) {
			// place mark beside the note
			xCoord = e->pos().x() + e->boundingRect().width();
			yCoord = e->pos().y() - 2;
		} else {
			// place mark above
			xCoord = e->pos().x();
			yCoord = qMin(e->pos().y(),e->drawableContext()->pos().y())-20*(j+1);
			j++;
		}

		if ( mark->markType()==CAMark::Articulation ) {
			xCoord = e->pos().x() + qRound(e->boundingRect().width()/2.0) - 3;
		}

		CADrawableMark *m = new CADrawableMark( mark, e->drawableContext() );

		if ( mark->markType()==CAMark::RehersalMark )
			m->setRehersalMarkNumber( _streamsRehersalMarks[ streamIdx ]++ );

		if (m->isHScalable() || m->isVScalable()) {
			_scalableElts << m;
		} else {
			_scoreView->addMElement( m );
			m->setPos(xCoord, yCoord);
		}
	}
}

/*!
	Updates all the context widths to fit the last drawable element width + offset.
*/
void CALayoutEngine::updateContextsWidth() {
	double maxWidth = 0;

	// first calculate the max width
	for (int i=0; i<_drawableContextMap.values().size(); i++) {
		CADrawableContext *dc = _drawableContextMap.values()[i];
		CADrawableMusElement *elt = dc->lastDrawableMusElement();
		if (!elt) { continue; }

		double width = elt->pos().x() + elt->boundingRect().width() + RIGHT_X_OFFSET;
		if ( width > maxWidth ) { maxWidth = width; }
	}

	// apply the max width to all contexts
	for (int i=0; i<_drawableContextMap.values().size(); i++) {
		CADrawableContext *dc = _drawableContextMap.values()[i];
		dc->setWidth( maxWidth );
	}
}
