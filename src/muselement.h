/** @file muselement.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef MUSELEMENT_H_
#define MUSELEMENT_H_

#include "drawable.h"

class CAContext;

class CAMusElement {
	public:
		CAMusElement(CAContext *context, int time, int length=0);
		~CAMusElement();
		
		/**
		 * enum CAMusElementType includes different types for describing the CAMusElement:
		 * - Note - A music element which represents CANote.
		 * - NoteBracket - A music element which represents CANoteBracket (the bracket which connects the stems).
		 * - Chord - A virtual music element which represents CAChord.
		 * - Rest - A music element which represents CARest.
		 * - BarLine - A music elemnet which represents CABarLine.
		 * - Clef - A music element which represents CAClef.
		 * - TimeSignature - A music element which represents CATimeSignature.
		 * - KeySignature - A music element which represents CAKeySignature.
		 * - Slur - A music element which represents CASlur.
		 * - Tie - A music element which represents CATie.
		 * - PhrazingSlur - A music element which represents CAPhrazingSlur.
		 * - ExpressionMarking - A music element which represents any technical text markings about how the score should be played - CAExpressionMarking (eg. Legato)
		 * - VolumeSign - A music element which represents any volue sign (forte, piano etc.).
		 * - Text - A music element which represents any text notes and authors additions to the score. (eg. These 3 measures still need to be fixed)
		 */
		enum CAMusElementType {
			None = 0,
			Note,
			NoteBracket,
			Chord,
			Rest,
			BarLine,
			Clef,
			TimeSignature,
			KeySignature,
			Slur,
			Tie,
			PhrazingSlur,
			ExpressionMarking,
			VolumeSign,
			Text
		};
		
		/**
		 * Return the music element type.
		 * See CAMusElement::enum CAMusElementType for details.
		 * 
		 * @return Music element type in CAMusElementType format.
		 */
		CAMusElementType musElementType() { return _musElementType; }
		
		/**
		 * Return the CAContext which this music element belongs too.
		 * 
		 * @return Pointer to the CAContext which this music element belongs too.
		 */
		CAContext *context() { return _context; }
		
		int timeStart() { return _timeStart; }
		int timeEnd() { return _timeStart + _timeLength; }
		int timeLength() { return _timeLength; }
		
		/**
		 * Set the new start time in the score for this music element.
		 * 
		 * @param time The new time in absolute sample units.
		 */
		void setTimeStart(int time) { _timeStart = time; }
		
		/**
		 * Set the new music element length in the score.
		 * 
		 * @param length The new length in absolue time sample units.
		 */
		void setTimeLength(int length) { _timeLength = length; }
		
		/**
		 * Return, if the current element is playable or not.
		 * 
		 * @return True, if the current element inherits CAPlayable class, false otherwise.
		 */
		bool isPlayable() { return _playable; }
	
	protected:
		CAMusElementType _musElementType;	///Stores the type of the music element. See CAMusElement::enum CAMusElementType for details.
		CAContext *_context;	///Pointer to the context which this music element belongs too.
		int _timeStart;		//Where does this music element exist in time (start)
		int _timeLength;	//How long is this music element in time
		bool _playable;	//Is the current element playable 
};

#endif /*MUSELEMENT_H_*/
