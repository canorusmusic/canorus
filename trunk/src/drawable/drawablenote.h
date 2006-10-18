/** @file drawable/drawablenote.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef DRAWABLENOTE_H_
#define DRAWABLENOTE_H_

#include "drawable/drawablemuselement.h"

class CANote;
class CADrawableAccidental;

class CADrawableNote : public CADrawableMusElement {
	public:
		/**
		 * Y coordinate represents the center of the notehead.
		 * X coordinate represents the left border of the notehead.
		 */
		CADrawableNote(CANote *note, CADrawableContext *drawableContext, int x, int y, bool shadowNote = false, CADrawableAccidental* acc=0);
		
		~CADrawableNote();
		
		void draw(QPainter *p, CADrawSettings s);
		
		inline CANote* note() { return (CANote*)_musElement; }
		
		CADrawableNote *clone();
		
		void setDrawLedgerLines(bool ledgerLines) { _drawLedgerLines = ledgerLines; }
		bool drawLedgerLines() { return _drawLedgerLines; }
		void setXPos(int xPos);
		void setYPos(int yPos);
		
		void setDrawableAccidental(CADrawableAccidental *acc) { _drawableAcc = acc; }
		CADrawableAccidental *drawableAccidental() { return _drawableAcc; }
		
	private:
		bool _drawLedgerLines;	///Are the ledger lines drawn or not. True when ledger lines needed, False when the note is inside the staff 
		bool _shadowNote;	///Is the current note shadow note?
		CADrawableAccidental *_drawableAcc;
		int _noteHeadWidth;
		static const int QUARTER_STEM_LENGTH = 22;
		static const int HALF_STEM_LENGTH = 24;
		static const int QUARTER_YPOS_DELTA = 21;
		static const int HALF_YPOS_DELTA = 23;
};

#endif /*DRAWABLENOTE_H_*/
