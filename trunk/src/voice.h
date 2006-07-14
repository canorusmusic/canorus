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
		 * Insert the clef before the first element which startTime is equal or greater than the clef's one.
		 * 
		 * @param clef Pointer to CAClef to be inserted.
		 */
		void insertClef(CAClef *clef);
		
		/**
		 * Insert the clef right before the given CAMusElement.
		 * 
		 * @param clef Pointer to CAClef to be inserted.
		 * @param eltAfter Pointer to CAMusElement the clef should be insered before. If eltAfter is 0, append the clef to the voice.
		 * @return True, if element was found and clef was inserted/appended, otherwise false.
		 */
		bool insertClefBefore(CAClef *clef, CAMusElement *eltAfter);
		
		QList<CAMusElement *>* musElementList() { return &_musElementList; }
		int lastTimeEnd() { return (_musElementList.size()?_musElementList.back()->timeEnd():0); }
	
	private:
		QList<CAMusElement *> _musElementList;
		CAStaff *_staff;	///Staff which this voice belongs to by default.
		int _voiceNumber;	///Number of the voice.
};

#endif /*VOICE_H_*/
