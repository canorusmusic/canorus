/** @file scripting/muselement.i
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

%{
#include "core/muselement.h"
%}

/**
 * Swig implementation of CAMusElement.
 */
%rename(MusElement) CAMusElement;
class CAMusElement {
	public:
		CAMusElement(CAContext *context, int time, int length=0);
		virtual CAMusElement* clone()=0;
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
			Undefined = 0,
			Note,
			NoteBracket,
			Rest,
			Barline,
			Clef,
			TimeSignature,
			KeySignature,
			Slur,
			Tie,
			PhrazingSlur,
			ExpressionMarking,
			VolumeSign,
			Text,
			FunctionMarking
		};
		
		/**
		 * Return the music element type.
		 * See CAMusElement::enum CAMusElementType for details.
		 * 
		 * @return Music element type in CAMusElementType format.
		 */
		CAMusElementType musElementType();
		
		/**
		 * Return the CAContext which this music element belongs too.
		 * 
		 * @return Pointer to the CAContext which this music element belongs too.
		 */
		CAContext *context();
		
		int timeStart();
		int timeEnd();
		int timeLength();
		
		/**
		 * Set the new start time in the score for this music element.
		 * 
		 * @param time The new time in absolute sample units.
		 */
		void setTimeStart(int time);
		
		/**
		 * Set the new music element length in the score.
		 * 
		 * @param length The new length in absolue time sample units.
		 */
		void setTimeLength(int length);
		
		/**
		 * Return, if the current element is playable or not.
		 * 
		 * @return True, if the current element inherits CAPlayable class, false otherwise.
		 */
		bool isPlayable();
		        
		const QString name();
		virtual int compare(CAMusElement *elt)=0;
};

