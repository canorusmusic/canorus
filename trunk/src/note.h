/** @file note.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef NOTE_H_
#define NOTE_H_

#include <QPainter>

#include "muselement.h"
#include "playable.h"

class CANote :  public CAMusElement, public CAPlayable {
	public:
		CANote(int length, int x, int y);
		void draw(QPainter *p, CADrawSettings s);
		CANote *clone();

	private:
		int _length;
};
#endif /*NOTE_H_*/

