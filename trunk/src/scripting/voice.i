/** @file scripting/voice.i
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

%{
#include "core/voice.h"
%}

/**
 * Swig implementation of CAVoice.
 */
%rename(Voice) CAVoice;
class CAVoice {
	public:
		CAVoice(CAStaff *staff, const QString name);
		inline CAStaff *staff();
		void clear();
		
		/**
		 * Insert the music element before the first element which startTime is equal or greater to the given element's one.
		 * 
		 * @param elt Pointer to CAMusElement to be inserted.
		 */
		void insertMusElement(CAMusElement *elt);
		
		/**
		 * Insert the CAMusElement right before the given CAMusElement.
		 * 
		 * @param elt Pointer to CAMusElement to be inserted.
		 * @param eltAfter Pointer to CAMusElement the given element should be inserted before. If eltAfter is 0, append the music element to the voice.
		 * @return True, if eltAfter was found and the elt was inserted/appended, otherwise false.
		 */
		bool insertMusElementBefore(CAMusElement *elt, CAMusElement *eltAfter);
		
		/**
		 * Insert the CAMusElement right after the given CAMusElement.
		 * 
		 * @param elt Pointer to CAMusElement to be inserted.
		 * @param eltBefore Pointer to CAMusElement the given element should be inserted after. If eltBefore is 0, append the music element to the voice.
		 * @return True, if eltAfter was found and the elt was inserted/appended, otherwise false.
		 */
		bool insertMusElementAfter(CAMusElement *elt, CAMusElement *eltBefore);

		/**
		 * Remove the given music element from the voice.
		 * This method doesn't delete its contents, but only the pointer.
		 * 
		 * @param elt Pointer to the element to be removed.
		 * @return True, if the element was found and removed, false otherwise.
		 */
		bool removeElement(CAMusElement *elt);
		
		int voiceNumber();
		void setVoiceNumber(int idx);
		
		int musElementCount();
		CAMusElement *musElementAt(int i);
		int indexOf(CAMusElement *elt);
		
		QList<CAMusElement*> *musElementList();	///Return the list of music elements
		int lastTimeEnd();
		int lastNotePitch();
		CANote::CANoteLength lastNoteLength();
		
		CAClef *getClef(CAMusElement *elt);
		
		const QString name();
		unsigned char midiChannel();
		unsigned char midiProgram();
	
		void setName(const QString name);
		void setMidiChannel(const unsigned char ch);
		void setMidiProgram(const unsigned char program);
};
