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

class CAMusElement {
	public:
		CAMusElement();
		
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
		 * - Text - A music element which represents any arbitrary text sign.
		 * - VolumeSign - A music element which represents any volue sign (forte, piano etc.).
		 */
		enum CAMusElementType {
			Note = 1,
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
			Text,
			VolumeSign
		};
		
		/**
		 * Return the music element type.
		 * See CAMusElement::enum CAMusElementType for details.
		 * 
		 * @return Music element type in CAMusElementType format.
		 */
		CAMusElementType musElementType() { return _musElementType; }
	
	protected:
		CAMusElementType _musElementType;	///Stores the type of the music element. See CAMusElement::enum CAMusElementType for details.
};

#endif /*MUSELEMENT_H_*/
