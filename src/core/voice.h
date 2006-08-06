/** @file voice.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef VOICE_H_
#define VOICE_H_

#include <QList>

#include "core/muselement.h"
#include "core/note.h"

class CAStaff;
class CAClef;

class CAVoice {
	public:
		CAVoice(CAStaff *staff, const QString name);
		inline CAStaff *staff() { return _staff; }
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
		 * Remove the given music element from the voice.
		 * This method doesn't delete its contents, but only the pointer.
		 * 
		 * @param elt Pointer to the element to be removed.
		 * @return True, if the element was found and removed, false otherwise.
		 */
		bool removeElement(CAMusElement *elt);
		
		int musElementCount() { return _musElementList.count(); }
		CAMusElement *musElementAt(int i) { return _musElementList[i]; }
		int indexOf(CAMusElement *elt) { return _musElementList.indexOf(elt); }
		
		QList<CAMusElement*> *musElementList() { return &_musElementList; }	///Return the list of music elements
		int lastTimeEnd() { return (_musElementList.size()?_musElementList.back()->timeEnd():0); }
		int lastNotePitch();
		CANote::CANoteLength lastNoteLength();
		
		CAClef *getClef(CAMusElement *elt);
		
		const QString name() { return _name; }
		unsigned char midiChannel() { return _midiChannel; }
		unsigned char midiProgram() { return _midiProgram; }
	
		void setName(const QString name) { _name = name; }
		void setMidiChannel(const unsigned char ch) { _midiChannel = ch; }
		void setMidiProgram(const unsigned char program) { _midiProgram = program; }
		
	private:
		void updateTimes(int idx);	///Update the musElements timeStarts, taking idx-th element as the inserted element.
		void updateTimes(int idx, int length);	///Update the musElements timeStarts from idx-th and on and substracting the length from every one. Called when an element is deleted and starTimes after it need to be updated.
		QList<CAMusElement *> _musElementList;
		CAStaff *_staff;	///Staff which this voice belongs to by default.
		int _voiceNumber;	///Voice number starting at 1.
		
		QString _name;
		
		////////////////////////////////////////////////
		//MIDI properties
		////////////////////////////////////////////////
		unsigned char _midiChannel;
		unsigned char _midiProgram;
};

#endif /*VOICE_H_*/
