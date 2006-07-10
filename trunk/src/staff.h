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

class CASheet;
class CAContext;
class CAVoice;

class CAStaff : public CAContext {
	public:
		CAStaff(CASheet *s, int x, int y);
		
		inline int numberOfLines() { return _numberOfLines; }
		inline void setNumberOfLines(int val) { _numberOfLines = val; }
		void clear();
		void draw(QPainter *, const CADrawSettings s);
		
	private:
		QList<CAVoice *> _voiceList;
		
		int _numberOfLines;
};

#endif /*STAFF_H_*/
