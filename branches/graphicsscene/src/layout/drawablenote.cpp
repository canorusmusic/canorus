/*!
	Copyright (c) 2006-2010, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPen>

#include "layout/drawablestaff.h"
#include "layout/drawablenote.h"
#include "layout/drawablecontext.h"
#include "layout/drawableaccidental.h"
#include "score/voice.h"
#include "score/staff.h"
#include "canorus.h"

const double CADrawableNote::HUNDREDTWENTYEIGHTH_STEM_LENGTH = 50;
const double CADrawableNote::SIXTYFOURTH_STEM_LENGTH = 43;
const double CADrawableNote::THIRTYSECOND_STEM_LENGTH = 37;
const double CADrawableNote::SIXTEENTH_STEM_LENGTH = 31;
const double CADrawableNote::EIGHTH_STEM_LENGTH = 31;
const double CADrawableNote::QUARTER_STEM_LENGTH = 31;
const double CADrawableNote::HALF_STEM_LENGTH = 31;
const double CADrawableNote::QUARTER_YPOS_DELTA = 21;
const double CADrawableNote::HALF_YPOS_DELTA = 23;

/*!
	Default constructor.

	\param x coordinate represents the left border of the notehead.
	\param y coordinate represents the center of the notehead.
*/
CADrawableNote::CADrawableNote(CANote *n, CADrawableContext *drawableContext, CADrawableAccidental *drawableAcc)
 : CADrawableMusElement(n, drawableContext, DrawableNote),
   _noteHead(0), _drawableAcc(drawableAcc), _stem(0), _flag(0) {
	double stemLength = 0;
	QString flagUpGlyphName, flagDownGlyphName, noteHeadGlyphName; // Feta glyph name for the notehead symbol and stems

	// Set note settings
	switch (n->playableLength().musicLength()) {
	case CAPlayableLength::HundredTwentyEighth:
		/// \todo Emmentaler font doesn't have 128th, 64th flag is drawn instead! Need to somehow compose the 128th flag? -Matevz
		stemLength = HUNDREDTWENTYEIGHTH_STEM_LENGTH;
		flagUpGlyphName = "flags.u7";
		flagDownGlyphName = "flags.d7";
		noteHeadGlyphName = "noteheads.s2";
		break;
	case CAPlayableLength::SixtyFourth:
		stemLength = SIXTYFOURTH_STEM_LENGTH;
		flagUpGlyphName = "flags.u6";
		flagDownGlyphName = "flags.d6";
		noteHeadGlyphName = "noteheads.s2";
		break;
	case CAPlayableLength::ThirtySecond:
		stemLength = THIRTYSECOND_STEM_LENGTH;
		flagUpGlyphName = "flags.u5";
		flagDownGlyphName = "flags.d5";
		noteHeadGlyphName = "noteheads.s2";
		break;
	case CAPlayableLength::Sixteenth:
		stemLength = SIXTEENTH_STEM_LENGTH;
		flagUpGlyphName = "flags.u4";
		flagDownGlyphName = "flags.d4";
		noteHeadGlyphName = "noteheads.s2";
		break;
	case CAPlayableLength::Eighth:
		stemLength = EIGHTH_STEM_LENGTH;
		flagUpGlyphName = "flags.u3";
		flagDownGlyphName = "flags.d3";
		noteHeadGlyphName = "noteheads.s2";
		break;
	case CAPlayableLength::Quarter:
		stemLength = QUARTER_STEM_LENGTH;
		noteHeadGlyphName = "noteheads.s2";
		break;
	case CAPlayableLength::Half:
		stemLength = HALF_STEM_LENGTH;
		noteHeadGlyphName = "noteheads.s1";
		break;
	case CAPlayableLength::Whole:
		noteHeadGlyphName = "noteheads.s0";
		break;
	case CAPlayableLength::Breve:
		noteHeadGlyphName = "noteheads.sM1";
		break;
	}

	// Draw notehead
	QFont font("Emmentaler");
	font.setPixelSize(qRound(35));

	_noteHead = new QGraphicsSimpleTextItem(QString(CACanorus::fetaCodepoint(noteHeadGlyphName)), this);
	_noteHead->setFont(font);
	_noteHead->setPos(0, -47);
	addToGroup(_noteHead);

	// Draw ledger lines
	updateLedgerLines();

	// Draw Dots
	double dotOffset = 3;
	double dotWidth  = 2.5;
	QBrush dotBrush(Qt::SolidPattern);
	for (int i=0; i<note()->playableLength().dotted(); i++) {
		QGraphicsEllipseItem *item = new QGraphicsEllipseItem(boundingRect().width() + dotOffset, -2.5, dotWidth, dotWidth, this);
		item->setBrush(dotBrush);
		addToGroup(item);
	}

	// Draw stem (and flag)
	if (note()->noteLength().musicLength() >= CAPlayableLength::Half) {
		QPen pen;
		pen.setCapStyle(Qt::RoundCap);
		pen.setWidthF(1.5);
		QFont flagFont("Emmentaler");
		flagFont.setPixelSize(qRound(35));

		if (note()->actualStemDirection() == CANote::StemUp) {
			_stem = new QGraphicsLineItem( _noteHead->boundingRect().width()-0.8, -1.5, _noteHead->boundingRect().width(), -stemLength, this );
			if(note()->noteLength().musicLength() >= CAPlayableLength::Eighth) {
				_flag = new QGraphicsSimpleTextItem( QString(CACanorus::fetaCodepoint(flagUpGlyphName)), this );
				_flag->setFont(flagFont);
				addToGroup(_flag);
				_flag->setPos( _noteHead->boundingRect().width(), -stemLength - 47 );
			}
		} else {
			_stem = new QGraphicsLineItem( 0.8, 1.5, 0.6, stemLength, this );
			if(note()->noteLength().musicLength() >= CAPlayableLength::Eighth) {
				_flag = new QGraphicsSimpleTextItem( QString(CACanorus::fetaCodepoint(flagDownGlyphName)), this );
				_flag->setFont(flagFont);
				addToGroup(_flag);
				_flag->setPos( 0.5, stemLength - 42 );
			}
		}
		addToGroup(_stem);
		_stem->setPen(pen);
	}
}

CADrawableNote::~CADrawableNote() {
}

/*!
	Updates the ledger lines positions and adds/removes them from the group.
 */
void CADrawableNote::updateLedgerLines() {
	// Cleans up the existing ledger lines, if any
	while (_ledgerLines.size()) {
		removeFromGroup(_ledgerLines[0]);
		delete _ledgerLines.takeFirst();
	}

	// Generate ledger lines, if needed
	if ( drawableContext() && drawableContext()->drawableContextType()==CADrawableContext::DrawableStaff &&
	     note() && note()->voice() && note()->voice()->staff() &&
	     ( (note()->notePosition() <= -2) ||	// note is below the staff
	       (note()->notePosition() >= note()->voice()->staff()->numberOfLines()*2)	// note is above the staff
	     )
	   ) {
	   	int direction = (note()->notePosition() > 0 ? 1 : -1);	// 1 falling, -1 rising
	   	double ledgerDist = static_cast<CADrawableStaff*>(drawableContext())->lineSpace();	// distance between the ledger lines - notehead height

	   	// draw ledger lines in direction from the notehead to staff
		double dy = ((note()->notePosition()%2)?(ledgerDist*0.5*direction):0);
		QPen pen;
		pen.setWidthF(1.0);

	   	for (int i=0;
	   	     i < ((note()->notePosition()*direction -
	   	          ((direction>0)?((note()->voice()->staff()->numberOfLines()-1)*2):0))/2);
	   	     i++
	   	    )
		{
			QGraphicsLineItem *line = new QGraphicsLineItem(-3.5, dy, _noteHead->boundingRect().width() + 2.5, dy, this);
			line->setPen(pen);
			_ledgerLines << line;
			addToGroup(line);
			dy += ledgerDist*direction;
		}
	}
}

/*!
	Sets/Unsets the drawable accidental in front of the note and adds/removes
	it from the group.
*/
void CADrawableNote::setDrawableAccidental( CADrawableAccidental *acc ) {
	if (acc==_drawableAcc) { return; }

	if (_drawableAcc) {
		removeFromGroup(_drawableAcc);
	}

	if (acc) {
		addToGroup(acc);
	}

	_drawableAcc = acc;
}

/*void CADrawableNote::draw(QPainter *p, CADrawSettings s) {
	QFont font("Emmentaler");
	font.setPixelSize(qRound(35*s.z));

	p->setPen(QPen(s.color));
	p->setFont(font);

	QPen pen;

	// Draw ledger lines
	if ( _drawLedgerLines && drawableContext() && drawableContext()->drawableContextType()==CADrawableContext::DrawableStaff &&
	     note() && note()->voice() && note()->voice()->staff() &&
	     ( (note()->notePosition() <= -2) ||	// note is below the staff
	       (note()->notePosition() >= note()->voice()->staff()->numberOfLines()*2)	// note is above the staff
	     )
	   ) {
	   	int direction = (note()->notePosition() > 0 ? 1 : -1);	// 1 falling, -1 rising
	   	double ledgerDist = static_cast<CADrawableStaff*>(drawableContext())->lineSpace()*s.z;	// distance between the ledger lines - notehead height

	   	// draw ledger lines in direction from the notehead to staff
		qreal ry = (direction==1)?_drawableContext->yPos():_drawableContext->yPos()+(_drawableContext->height());
		ry -= s.worldY;
		ry *= s.z;
		QPen pen(s.color);
		pen.setWidthF(1.0*s.z);
		p->setPen(pen);
	   	for (int i=0;
	   	     i < ((note()->notePosition()*direction -
	   	          ((direction>0)?((note()->voice()->staff()->numberOfLines()-1)*2):0))/2);
	   	     ++i
	   	    )
		{
			ry -= ledgerDist*direction;
			p->drawLine(qRound(s.x - 4*s.z), qRound(ry), qRound(s.x + (_noteHeadWidth + 4)*s.z), qRound(ry));
		}
	}

	// Draw notehead
	s.y += height()*s.z/2;
	p->drawText(s.x, qRound(s.y), QString(CACanorus::fetaCodepoint(_noteHeadGlyphName)));

	if (note()->noteLength().musicLength() >= CAPlayableLength::Half) {
		// Draw stem and flag
		pen.setWidthF(_penWidth*s.z);
		pen.setCapStyle(Qt::RoundCap);
		pen.setColor(s.color);
		p->setPen(pen);
		if (_stemDirection == CANote::StemUp) {
			s.x+=qRound(_noteHeadWidth*s.z); // increase X-offset before drawing the stem
			p->drawLine(s.x, qRound(s.y-1*s.z), s.x, s.y-qRound(_stemLength*s.z));
			if(note()->noteLength().musicLength() >= CAPlayableLength::Eighth) {
				p->drawText(qRound(s.x+0.6*s.z),qRound(s.y - _stemLength*s.z),QString(CACanorus::fetaCodepoint(_flagUpGlyphName)));
				s.x+=qRound(6*s.z); // additional X-offset for dots because of the flag on the right
			}
		} else {
			s.x+=qRound(0.6*s.z);
			p->drawLine(s.x, qRound(s.y+1*s.z), s.x, s.y+qRound(_stemLength*s.z));
			if(note()->noteLength().musicLength() >= CAPlayableLength::Eighth) {
				p->drawText(qRound(s.x+0.4*s.z),qRound(s.y + (_stemLength+5)*s.z),QString(CACanorus::fetaCodepoint(_flagDownGlyphName)));
			}
			s.x+=qRound(_noteHeadWidth*s.z); // increase X-offset after drawing the stem
		}
	} else {
		s.x+=qRound(_noteHeadWidth*s.z); // increase X-offset for drawing the dots
	}

	// Draw Dots
	float delta=4*s.z;
	for (int i=0; i<note()->playableLength().dotted(); i++) {
		pen.setWidth(qRound(2.7*s.z) + 1);
		pen.setCapStyle(Qt::RoundCap);
		pen.setColor(s.color);
		p->setPen(pen);
		p->drawPoint(qRound(s.x + delta), qRound(s.y - 1.7*s.z));
		delta += 4*s.z;
	}

	s.x += qRound(delta);
}
*/
