/** @file staff.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef STAFF_H_
#define STAFF_H_

#include <QList>

class QPainter;

#include "context.h"
#include "clef.h"

class CASheet;
class CAContext;
class CAVoice;

class CAStaff : public CAContext {
	public:
		CAStaff(CASheet *s);
		
		inline int numberOfLines() { return _numberOfLines; }
		inline void setNumberOfLines(int val) { _numberOfLines = val; }
		void clear();
		
		/**
		 * Return the end of the last music element in the staff.
		 * 
		 * @return End of the last music element in the staff.
		 */
		int lastTimeEnd();
		
		/**
		 * Return the number of voices.
		 * 
		 * @return Number of voices.
		 */
		int voiceCount() { return _voiceList.size(); }
		
		/**
		 * Return voice with the specified index.
		 * 
		 * @return Pointer to the voice with the given index.
		 */
		CAVoice *voiceAt(int i) { return _voiceList[i]; }
		
		/**
		 * Add a clef to the staff at certain time and return the new clef pointer.
		 * 
		 * @param clefType Type of the clef - see CAClef::CAClefType.
		 * @param timeStart The logical time where clef is.
		 * @return Pointer to the newly created clef.
		 */
		CAClef *insertClef(CAClef::CAClefType clefType, int timeStart);
		
		/**
		 * Add a clef to the staff before certain element and return the new clef pointer.
		 * 
		 * @param clefType Type of the clef - see CAClef::CAClefType.
		 * @param eltAfter The music element before which the clef should be inserted. If eltAfter is 0, append the clef to the voices.
		 * @return Pointer to the newly created clef.
		 */
		CAClef *insertClefBefore(CAClef::CAClefType clefType, CAMusElement *eltAfter);
		
	private:
		QList<CAVoice *> _voiceList;
		
		int _numberOfLines;
};

#endif /*STAFF_H_*/
