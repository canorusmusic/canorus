/*!
	Copyright (c) 2006-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef DRAWABLENOTE_H_
#define DRAWABLENOTE_H_

#include "layout/drawablemuselement.h"
#include "score/note.h"

class CANote;
class CADrawableAccidental;

class CADrawableNote : public CADrawableMusElement {
	public:
		CADrawableNote(CANote *note, CADrawableContext *drawableContext, double x, double y, bool shadowNote = false, CADrawableAccidental* acc=0);

		~CADrawableNote();

		void draw(QPainter *p, CADrawSettings s);

		inline CANote* note() { return static_cast<CANote*>(_musElement); }

		CADrawableNote *clone(CADrawableContext* newContext = 0);

		void setDrawLedgerLines(bool ledgerLines) { _drawLedgerLines = ledgerLines; }
		bool drawLedgerLines() { return _drawLedgerLines; }

		void setDrawableAccidental(CADrawableAccidental *acc) { _drawableAcc = acc; }
		CADrawableAccidental *drawableAccidental() { return _drawableAcc; }

	private:
		bool _drawLedgerLines;	///Are the ledger lines drawn or not. True when ledger lines needed, False when the note is inside the staff
		bool _shadowNote;	///Is the current note shadow note?
		CADrawableAccidental *_drawableAcc;
		CANote::CAStemDirection _stemDirection;	/// This value is StemUp or StemDown only, no StemPreferred or StemNeutral present. We generate this on CADrawableNote constructor.
		double _stemLength;
		double _noteHeadWidth;
		double _penWidth; // pen width for stem
		QString _noteHeadGlyphName; // Feta glyph name for the notehead symbol.
		QString _flagUpGlyphName; // likewise for stem flags
		QString _flagDownGlyphName;
		static const double HUNDREDTWENTYEIGHTH_STEM_LENGTH;
		static const double SIXTYFOURTH_STEM_LENGTH;
		static const double THIRTYSECOND_STEM_LENGTH;
		static const double SIXTEENTH_STEM_LENGTH;
		static const double EIGHTH_STEM_LENGTH;
		static const double QUARTER_STEM_LENGTH;
		static const double HALF_STEM_LENGTH;
		static const double QUARTER_YPOS_DELTA;
		static const double HALF_YPOS_DELTA;
};

#endif /* DRAWABLENOTE_H_ */
