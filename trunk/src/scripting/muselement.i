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
		
		enum CAMusElementType {
			Undefined = 0,
			Note,
			Rest,
			Barline,
			Clef,
			TimeSignature,
			KeySignature,
			Slur,
			Syllable,
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
		
	static const QString musElementTypeToString(CAMusElementType);
	static CAMusElementType musElementTypeFromString(const QString);
};

