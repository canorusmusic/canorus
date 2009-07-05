/*!
	Copyright (c) 2006-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPen>
#include <QPainter>

#include "layout/drawablebarline.h"
#include "layout/drawablestaff.h"

#include "score/barline.h"

const float CADrawableBarline::SPACE_BETWEEN_BARLINES = 3;

const float CADrawableBarline::BARLINE_WIDTH = 1.5;
const float CADrawableBarline::DOTTED_BARLINE_WIDTH = 2;
const float CADrawableBarline::BOLD_BARLINE_WIDTH = 4;
const float CADrawableBarline::REPEAT_DOTS_WIDTH = 3;

CADrawableBarline::CADrawableBarline(CABarline *m, CADrawableStaff *staff, double x, double y)
 : CADrawableMusElement(m, staff, x, y) {
 	setDrawableMusElementType( CADrawableMusElement::DrawableBarline );

 	switch (m->barlineType()) {
 		case CABarline::Single:
 			setWidth( BARLINE_WIDTH );
 			break;
 		case CABarline::Double:
 			setWidth( 2*BARLINE_WIDTH + SPACE_BETWEEN_BARLINES );
 			break;
 		case CABarline::End:
 			setWidth( BARLINE_WIDTH + SPACE_BETWEEN_BARLINES + BOLD_BARLINE_WIDTH );
 			break;
 		case CABarline::RepeatOpen:
 			setWidth( BOLD_BARLINE_WIDTH + 2*SPACE_BETWEEN_BARLINES + BARLINE_WIDTH + REPEAT_DOTS_WIDTH );
 			break;
 		case CABarline::RepeatClose:
 			setWidth( REPEAT_DOTS_WIDTH + 2*SPACE_BETWEEN_BARLINES + BARLINE_WIDTH + BOLD_BARLINE_WIDTH );
 			break;
 		case CABarline::RepeatCloseOpen:
 			setWidth( 2*REPEAT_DOTS_WIDTH + 3*SPACE_BETWEEN_BARLINES + BOLD_BARLINE_WIDTH + 2*BARLINE_WIDTH);
 			break;
 		case CABarline::Dotted:
 			setWidth( DOTTED_BARLINE_WIDTH );
 			break;
 	}

	setHeight( staff->height() );
	setNeededSpaceWidth(4);
}

CADrawableBarline::~CADrawableBarline() {
}

void CADrawableBarline::draw(QPainter *p, CADrawSettings s) {
	QPen pen(s.color);
	pen.setCapStyle(Qt::FlatCap);
	QBrush brush(s.color, Qt::SolidPattern);
	p->setBrush(brush);

	switch (barline()->barlineType()) {
		case CABarline::Single:
			// draw single barline
			pen.setWidth(qRound(BARLINE_WIDTH*s.z));
			p->setPen(pen);
			p->drawLine(qRound(s.x), s.y,
			            qRound(s.x), qRound(s.y + height()*s.z));
			break;
		case CABarline::Double:
			// draw double barline
			pen.setWidth((int)(BARLINE_WIDTH*s.z));
			p->setPen(pen);
			p->drawLine(qRound(s.x), s.y,
			            qRound(s.x), qRound(s.y + height()*s.z));
			p->drawLine(qRound(s.x + BARLINE_WIDTH*s.z + SPACE_BETWEEN_BARLINES*s.z), s.y,
			            qRound(s.x + BARLINE_WIDTH*s.z + SPACE_BETWEEN_BARLINES*s.z), qRound(s.y + height()*s.z));
			break;
		case CABarline::End:
			// draw thin barline
			pen.setWidth(qRound(BARLINE_WIDTH*s.z));
			p->setPen(pen);
			p->drawLine(qRound(s.x), s.y,
			            qRound(s.x), qRound(s.y + height()*s.z));
			// draw bold barline
			pen.setWidth(qRound(BOLD_BARLINE_WIDTH*s.z));
			p->setPen(pen);
			p->drawLine(qRound(s.x + BARLINE_WIDTH*s.z + SPACE_BETWEEN_BARLINES*s.z), s.y,
			            qRound(s.x + BARLINE_WIDTH*s.z + SPACE_BETWEEN_BARLINES*s.z), qRound(s.y + height()*s.z));
			break;
		case CABarline::RepeatOpen:
			pen.setWidth(qRound(BOLD_BARLINE_WIDTH*s.z));
			p->setPen(pen);
			// draw bold barline
			p->drawLine( qRound(s.x), s.y,
			             qRound(s.x), qRound(s.y + height()*s.z));
			s.x += qRound((BOLD_BARLINE_WIDTH/2 + SPACE_BETWEEN_BARLINES + BARLINE_WIDTH/2)*s.z);
			// draw single barline
			pen.setWidth(qRound(BARLINE_WIDTH*s.z));
			p->setPen(pen);
			p->drawLine(qRound(s.x), s.y,
			            qRound(s.x), qRound(s.y + height()*s.z));
			s.x += qRound((BARLINE_WIDTH/2 + SPACE_BETWEEN_BARLINES)*s.z);
			// draw upper dot
			p->drawEllipse( qRound(s.x),
			                qRound(s.y + 4*SPACE_BETWEEN_BARLINES*s.z),
			                qRound(REPEAT_DOTS_WIDTH*s.z), qRound(REPEAT_DOTS_WIDTH*s.z) );
			// draw lower dot
			p->drawEllipse( qRound(s.x),
			                qRound(s.y + (height()-REPEAT_DOTS_WIDTH-4*SPACE_BETWEEN_BARLINES)*s.z),
			                qRound(REPEAT_DOTS_WIDTH*s.z), qRound(REPEAT_DOTS_WIDTH*s.z) );
			break;
		case CABarline::RepeatClose:
			pen.setWidth(qRound(BARLINE_WIDTH*s.z));
			p->setPen(pen);
			// draw upper dot
			p->drawEllipse( qRound(s.x),
			                qRound(s.y + 4*SPACE_BETWEEN_BARLINES*s.z),
			                qRound(REPEAT_DOTS_WIDTH*s.z), qRound(REPEAT_DOTS_WIDTH*s.z) );
			// draw lower dot
			p->drawEllipse( qRound(s.x),
			                qRound(s.y + (height()-REPEAT_DOTS_WIDTH-4*SPACE_BETWEEN_BARLINES)*s.z),
			                qRound(REPEAT_DOTS_WIDTH*s.z), qRound(REPEAT_DOTS_WIDTH*s.z) );
			s.x += qRound((REPEAT_DOTS_WIDTH + SPACE_BETWEEN_BARLINES + BARLINE_WIDTH/2)*s.z);
			// draw single barline
			p->drawLine(qRound(s.x), s.y,
			            qRound(s.x), qRound(s.y + height()*s.z));
			s.x += qRound((BARLINE_WIDTH/2 + SPACE_BETWEEN_BARLINES + BOLD_BARLINE_WIDTH/2)*s.z);
			// draw bold barline
			pen.setWidth(qRound(BOLD_BARLINE_WIDTH*s.z));
			p->setPen(pen);
			p->drawLine( qRound(s.x), s.y,
			             qRound(s.x), qRound(s.y + height()*s.z));
			break;
		case CABarline::RepeatCloseOpen:
			pen.setWidth(qRound(BARLINE_WIDTH*s.z));
			p->setPen(pen);
			// draw upper dot
			p->drawEllipse( qRound(s.x),
			                qRound(s.y + 4*SPACE_BETWEEN_BARLINES*s.z),
			                qRound(REPEAT_DOTS_WIDTH*s.z), qRound(REPEAT_DOTS_WIDTH*s.z) );
			// draw lower dot
			p->drawEllipse( qRound(s.x),
			                qRound(s.y + (height()-REPEAT_DOTS_WIDTH-4*SPACE_BETWEEN_BARLINES)*s.z),
			                qRound(REPEAT_DOTS_WIDTH*s.z), qRound(REPEAT_DOTS_WIDTH*s.z) );
			s.x += qRound((REPEAT_DOTS_WIDTH + SPACE_BETWEEN_BARLINES + BARLINE_WIDTH/2)*s.z);
			// draw single barline
			p->drawLine(qRound(s.x), s.y,
			            qRound(s.x), qRound(s.y + height()*s.z));
			s.x += qRound((BARLINE_WIDTH/2 + SPACE_BETWEEN_BARLINES + BOLD_BARLINE_WIDTH/2)*s.z);
			// draw bold barline
			pen.setWidth(qRound(BOLD_BARLINE_WIDTH*s.z));
			p->setPen(pen);
			p->drawLine( qRound(s.x), s.y,
			             qRound(s.x), qRound(s.y + height()*s.z));
			s.x += qRound((BOLD_BARLINE_WIDTH/2 + SPACE_BETWEEN_BARLINES + BARLINE_WIDTH/2)*s.z);
			pen.setWidth(qRound(BARLINE_WIDTH*s.z));
			p->setPen(pen);
			// draw single barline
			p->drawLine(qRound(s.x), s.y,
			            qRound(s.x), qRound(s.y + height()*s.z));
			s.x += qRound((BARLINE_WIDTH/2 + SPACE_BETWEEN_BARLINES)*s.z);
			// draw upper dot
			p->drawEllipse( qRound(s.x),
			                qRound(s.y + 4*SPACE_BETWEEN_BARLINES*s.z),
			                qRound(REPEAT_DOTS_WIDTH*s.z), qRound(REPEAT_DOTS_WIDTH*s.z) );
			// draw lower dot
			p->drawEllipse( qRound(s.x),
			                qRound(s.y + (height()-REPEAT_DOTS_WIDTH-4*SPACE_BETWEEN_BARLINES)*s.z),
			                qRound(REPEAT_DOTS_WIDTH*s.z), qRound(REPEAT_DOTS_WIDTH*s.z) );

			break;
		case CABarline::Dotted:
			pen.setStyle( Qt::DotLine );
			pen.setCapStyle(Qt::RoundCap);
			pen.setWidth(qRound(DOTTED_BARLINE_WIDTH*s.z));
			p->setPen(pen);
			p->drawLine( qRound(s.x), s.y,
			             qRound(s.x), qRound(s.y + height()*s.z) );
			break;
	}

	p->setBrush(QBrush(s.color, Qt::NoBrush)); // reset the painter's brush
}

CADrawableBarline* CADrawableBarline::clone(CADrawableContext* newContext) {
	return new CADrawableBarline((CABarline*)_musElement, (CADrawableStaff*)((newContext)?newContext:_drawableContext), _xPos, _yPos);
}
