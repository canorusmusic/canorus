/*!
	Copyright (c) 2006-2010, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPen>

#include "layout/drawablebarline.h"
#include "layout/drawablestaff.h"

#include "score/barline.h"

const double CADrawableBarline::SPACE_BETWEEN_BARLINES = 3.0;

const double CADrawableBarline::BARLINE_WIDTH = 1.5;
const double CADrawableBarline::DOTTED_BARLINE_WIDTH = 2.0;
const double CADrawableBarline::BOLD_BARLINE_WIDTH = 4.0;
const double CADrawableBarline::REPEAT_DOTS_Y_OFFSET = 8.0;
const double CADrawableBarline::REPEAT_DOTS_WIDTH = 3.0;

CADrawableBarline::CADrawableBarline(CABarline *m, CADrawableStaff *staff)
 : CADrawableMusElement(m, staff, DrawableBarline) {
	QPen pen;
	pen.setCapStyle(Qt::FlatCap);

	double height = staff->boundingRect().height();

	switch (barline()->barlineType()) {
	case CABarline::Single:
		pen.setWidth(BARLINE_WIDTH);
		_lines << new QGraphicsLineItem(0, 0, 0, height, this);
		_lines.back()->setPen(pen);
		break;
	case CABarline::Double:
		pen.setWidth(BARLINE_WIDTH);
		_lines << new QGraphicsLineItem(0, 0, 0, height, this);
		_lines.back()->setPen(pen);
		_lines << new QGraphicsLineItem(BARLINE_WIDTH+SPACE_BETWEEN_BARLINES, 0,
		                                BARLINE_WIDTH+SPACE_BETWEEN_BARLINES, height, this);
		_lines.back()->setPen(pen);
		break;
	case CABarline::End:
		pen.setWidth(BARLINE_WIDTH);
		_lines << new QGraphicsLineItem(0, 0, 0, height, this);
		_lines.back()->setPen(pen);

		pen.setWidth(BOLD_BARLINE_WIDTH);
		_lines << new QGraphicsLineItem(BARLINE_WIDTH+SPACE_BETWEEN_BARLINES, 0,
		                                BARLINE_WIDTH+SPACE_BETWEEN_BARLINES, height, this);
		_lines.back()->setPen(pen);
		break;
	case CABarline::RepeatOpen:
		pen.setWidth(BOLD_BARLINE_WIDTH);
		_lines << new QGraphicsLineItem(0, 0, 0, height, this);
		_lines.back()->setPen(pen);

		pen.setWidth(BARLINE_WIDTH);
		_lines << new QGraphicsLineItem(0.5*BOLD_BARLINE_WIDTH + SPACE_BETWEEN_BARLINES + 0.5*BARLINE_WIDTH, 0,
		                                0.5*BOLD_BARLINE_WIDTH + SPACE_BETWEEN_BARLINES + 0.5*BARLINE_WIDTH, height, this);
		_lines.back()->setPen(pen);

		_dots << new QGraphicsEllipseItem(0.5*BOLD_BARLINE_WIDTH + 2*SPACE_BETWEEN_BARLINES + BARLINE_WIDTH, REPEAT_DOTS_Y_OFFSET,
		                                  REPEAT_DOTS_WIDTH, REPEAT_DOTS_WIDTH, this);
		_dots << new QGraphicsEllipseItem(0.5*BOLD_BARLINE_WIDTH + 2*SPACE_BETWEEN_BARLINES + BARLINE_WIDTH, height-REPEAT_DOTS_Y_OFFSET,
		                                  REPEAT_DOTS_WIDTH, REPEAT_DOTS_WIDTH, this);
		break;
	case CABarline::RepeatClose:
		_dots << new QGraphicsEllipseItem(0, REPEAT_DOTS_Y_OFFSET,
		                                  0, REPEAT_DOTS_WIDTH, this);
		_dots << new QGraphicsEllipseItem(0, height-REPEAT_DOTS_Y_OFFSET,
		                                  0, REPEAT_DOTS_WIDTH, this);

		pen.setWidth(BARLINE_WIDTH);
		_lines << new QGraphicsLineItem(REPEAT_DOTS_WIDTH + SPACE_BETWEEN_BARLINES + 0.5*BARLINE_WIDTH, 0,
		                                REPEAT_DOTS_WIDTH + SPACE_BETWEEN_BARLINES + 0.5*BARLINE_WIDTH, height, this);
		_lines.back()->setPen(pen);

		pen.setWidth(BOLD_BARLINE_WIDTH);
		_lines << new QGraphicsLineItem(REPEAT_DOTS_WIDTH + 2*SPACE_BETWEEN_BARLINES + 0.5*BOLD_BARLINE_WIDTH, 0,
		                                REPEAT_DOTS_WIDTH + 2*SPACE_BETWEEN_BARLINES + 0.5*BOLD_BARLINE_WIDTH, height, this);
		_lines.back()->setPen(pen);
		break;
	case CABarline::RepeatCloseOpen:
		_dots << new QGraphicsEllipseItem(0, REPEAT_DOTS_Y_OFFSET,
		                                  0, REPEAT_DOTS_WIDTH, this);
		_dots << new QGraphicsEllipseItem(0, height-REPEAT_DOTS_Y_OFFSET,
		                                  0, REPEAT_DOTS_WIDTH, this);

		pen.setWidth(BARLINE_WIDTH);
		_lines << new QGraphicsLineItem(REPEAT_DOTS_WIDTH + SPACE_BETWEEN_BARLINES + 0.5*BARLINE_WIDTH, 0,
		                               REPEAT_DOTS_WIDTH + SPACE_BETWEEN_BARLINES + 0.5*BARLINE_WIDTH, height, this);
		_lines.back()->setPen(pen);

		pen.setWidth(BOLD_BARLINE_WIDTH);
		_lines << new QGraphicsLineItem(REPEAT_DOTS_WIDTH + 2*SPACE_BETWEEN_BARLINES + BARLINE_WIDTH + 0.5*BOLD_BARLINE_WIDTH, 0,
		                                REPEAT_DOTS_WIDTH + 2*SPACE_BETWEEN_BARLINES + BARLINE_WIDTH + 0.5*BOLD_BARLINE_WIDTH, height, this);
		_lines.back()->setPen(pen);

		pen.setWidth(BARLINE_WIDTH);
		_lines << new QGraphicsLineItem(REPEAT_DOTS_WIDTH + 3*SPACE_BETWEEN_BARLINES + 1.5*BARLINE_WIDTH + BOLD_BARLINE_WIDTH, 0,
		                               REPEAT_DOTS_WIDTH + 3*SPACE_BETWEEN_BARLINES + 1.5*BARLINE_WIDTH + BOLD_BARLINE_WIDTH, height, this);
		_lines.back()->setPen(pen);

		_dots << new QGraphicsEllipseItem(REPEAT_DOTS_WIDTH + 4*SPACE_BETWEEN_BARLINES + 2*BARLINE_WIDTH + BOLD_BARLINE_WIDTH, REPEAT_DOTS_Y_OFFSET,
		                                  REPEAT_DOTS_WIDTH + 4*SPACE_BETWEEN_BARLINES + 2*BARLINE_WIDTH + BOLD_BARLINE_WIDTH, REPEAT_DOTS_WIDTH, this);
		_dots << new QGraphicsEllipseItem(REPEAT_DOTS_WIDTH + 4*SPACE_BETWEEN_BARLINES + 2*BARLINE_WIDTH + BOLD_BARLINE_WIDTH, height-REPEAT_DOTS_Y_OFFSET,
		                                  REPEAT_DOTS_WIDTH + 4*SPACE_BETWEEN_BARLINES + 2*BARLINE_WIDTH + BOLD_BARLINE_WIDTH, REPEAT_DOTS_WIDTH, this);
		break;
	case CABarline::Dotted:
		pen.setStyle( Qt::DotLine );
		pen.setCapStyle(Qt::RoundCap);
		pen.setWidth(DOTTED_BARLINE_WIDTH);
		_lines << new QGraphicsLineItem(0, 0, 0, height);
		_lines.back()->setPen(pen);
		break;
	}

	for (int i=0; i<_lines.size(); i++) {
		addToGroup(_lines[i]);
	}
	for (int i=0; i<_dots.size(); i++) {
		addToGroup(_dots[i]);
	}
}

CADrawableBarline::~CADrawableBarline() {
}

/*void CADrawableBarline::draw(QPainter *p, CADrawSettings s) {
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
}*/

CADrawableBarline* CADrawableBarline::clone(CADrawableContext* newContext) {
	return new CADrawableBarline(barline(), static_cast<CADrawableStaff*>((newContext)?newContext:_drawableContext));
}
