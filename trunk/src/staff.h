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
class CANote;

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
		 * Insert a sign to the staff at certain time.
		 * This method inserts the sign to all voices!
		 * This is the difference between inserting a sign or a playable note - note is present in a single voice, sign in both.
		 * 
		 * @param sign Pointer to the already created CAMusElement object about to be added.
		 */
		void insertSign(CAMusElement *sign);
		
		/**
		 * Insert any sign (clef, siganutres, barline etc.) before the given music element.
		 * This method inserts the sign to all voices!
		 * This is the difference between inserting a sign or a playable note - note is present in a single voice, sign in both.
		 * 
		 * @param sign Pointer to the already created CAMusElement object about to be added.
		 * @param eltAfter The music element before which the sign should be inserted. The element should be present in all voices! If eltAfter is 0, append the sign to the voices.
		 * @return True, if a sign was inserted/appended, false if the eltAfter wasn't found.
		 */
		bool insertSignBefore(CAMusElement *sign, CAMusElement *eltAfter);
		
		void insertNote(CANote *note);
		bool insertNoteBefore(CANote *note, CAMusElement *eltAfter);
		
	private:
		QList<CAVoice *> _voiceList;
		
		int _numberOfLines;
};

#endif /*STAFF_H_*/
