/*!
	Copyright (c) 2006-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef DRAWABLENOTE_H_
#define DRAWABLENOTE_H_

#include "drawable/drawablemuselement.h"
#include "core/note.h"

class CANote;
class CADrawableAccidental;

class CADrawableNote : public CADrawableMusElement {
	public:
		CADrawableNote(CANote *note, CADrawableContext *drawableContext, int x, int y, bool shadowNote = false, CADrawableAccidental* acc=0);
		
		~CADrawableNote();
		
		void draw(QPainter *p, CADrawSettings s);
		
		inline CANote* note() { return (CANote*)_musElement; }
		
		CADrawableNote *clone(CADrawableContext* newContext = 0);
		
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
		CANote::CAStemDirection _stemDirection;	/// This value is StemUp or StemDown only, no StemPreferred or StemNeutral present. We generate this on CADrawableNote constructor.
		int _stemLength;
		int _noteHeadWidth;
		float _penWidth; // pen width for stem
		QString _noteHeadGlyphName; // Feta glyph name for the notehead symbol.
		QString _flagUpGlyphName; // likewise for stem flags 
		QString _flagDownGlyphName;
		static const int HUNDREDTWENTYEIGHTH_STEM_LENGTH;
		static const int SIXTYFOURTH_STEM_LENGTH;
		static const int THIRTYSECOND_STEM_LENGTH;
		static const int SIXTEENTH_STEM_LENGTH;
		static const int EIGHTH_STEM_LENGTH;
		static const int QUARTER_STEM_LENGTH;
		static const int HALF_STEM_LENGTH;
		static const int QUARTER_YPOS_DELTA;
		static const int HALF_YPOS_DELTA;
};

#endif /* DRAWABLENOTE_H_ */
