/** @file core/muselement.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef MUSELEMENT_H_
#define MUSELEMENT_H_

#include <QString>

class CAContext;

class CAMusElement {
	public:
		CAMusElement(CAContext *context, int time, int length=0);
		virtual ~CAMusElement();
		
		virtual CAMusElement* clone()=0;
		
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
		CAMusElementType musElementType() { return _musElementType; }
		
		/**
		 * Return the CAContext which this music element belongs too.
		 * 
		 * @return Pointer to the CAContext which this music element belongs too.
		 */
		CAContext *context() { return _context; }
		
		int timeStart() { return _timeStart; }

		/**
		 * Set the new start time in the score for this music element.
		 * 
		 * @param time The new time in absolute sample units.
		 */
		void setTimeStart(int time) { _timeStart = time; }

		int timeEnd() { return _timeStart + _timeLength; }

		int timeLength() { return _timeLength; }

		/**
		 * Set the new music element length in the score.
		 * 
		 * @param length The new length in absolue time sample units.
		 */
		void setTimeLength(int length) { _timeLength = length; }

		const QString name() { return _name; }
		void setName(const QString name) { _name = name; }		
		
		/**
		 * Return, if the current element is playable or not.
		 * 
		 * @return True, if the current element inherits CAPlayable class, false otherwise.
		 */
		bool isPlayable() { return _playable; }
		
		/**
		 * Compare the given music element with this one.
		 * This is used when music element is present in both voices at the same time. If comparison is correct, then the objects are merged internally inside a staff.
		 * 
		 * @param elt Pointer to the music element to compare with.
		 * @return Return -1, if the music elements are of different types. Return 0 if the music elements match. Otherwise, return a number of attributes where music elements differ.
		 */
		virtual int compare(CAMusElement *elt) = 0;

	protected:
		CAMusElementType _musElementType;	///Stores the type of the music element. See CAMusElement::enum CAMusElementType for details.
		CAContext *_context;	///Pointer to the context which this music element belongs too.
		int _timeStart;		///Where does this music element exist in time (start)
		int _timeLength;	///How long is this music element in time
		bool _playable;	///Is the current element playable 
		QString _name;	///Specific name of this music element (optional)
};

#endif /*MUSELEMENT_H_*/
