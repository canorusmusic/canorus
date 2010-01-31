/*!
	Copyright (c) 2006-2010, Matevž Jekovec, Canorus development team
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
		CADrawableNote(CANote *note, CADrawableContext *drawableContext, CADrawableAccidental *acc=0);

		~CADrawableNote();

		inline CANote* note() { return static_cast<CANote*>(_musElement); }

		CADrawableNote *clone(CADrawableContext* newContext = 0);

		void updateLedgerLines();
		void setDrawableAccidental(CADrawableAccidental *acc);
		CADrawableAccidental *drawableAccidental() { return _drawableAcc; }

	private:
		QGraphicsSimpleTextItem  *_noteHead;
		CADrawableAccidental     *_drawableAcc;
		QList<QGraphicsLineItem*> _ledgerLines;   // Generated ledger lines
		QGraphicsLineItem        *_stem;
		QGraphicsSimpleTextItem  *_flag;

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
