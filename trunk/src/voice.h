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

#include "muselement.h"

class CAStaff;
class CAClef;

class CAVoice {
	public:
		CAVoice(CAStaff *staff);
		CAStaff *staff() { return _staff; }
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
		
		QList<CAMusElement *>* musElementList() { return &_musElementList; }
		int lastTimeEnd() { return (_musElementList.size()?_musElementList.back()->timeEnd():0); }
	
		CAClef *getClef(CAMusElement *elt);
	
	private:
		QList<CAMusElement *> _musElementList;
		CAStaff *_staff;	///Staff which this voice belongs to by default.
		int _voiceNumber;	///Number of the voice.
};

#endif /*VOICE_H_*/
