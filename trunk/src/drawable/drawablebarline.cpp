/** @file drawablebarline.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QPen>
#include <QPainter>

#include "drawable/drawablebarline.h"
#include "drawable/drawablestaff.h"

#include "core/barline.h"

const float CADrawableBarline::SPACE_BETWEEN_BARLINES = 3;

const float CADrawableBarline::BARLINE_WIDTH = 1.5;
const float CADrawableBarline::DOTTED_BARLINE_WIDTH = 2;
const float CADrawableBarline::BOLD_BARLINE_WIDTH = 4;
const float CADrawableBarline::REPEAT_DOTS_WIDTH = 3;

CADrawableBarline::CADrawableBarline(CABarline *m, CADrawableStaff *staff, int x, int y)
 : CADrawableMusElement(m, staff, x, y) {
 	_drawableMusElementType = CADrawableMusElement::DrawableBarline;
 	
 	switch (m->barlineType()) {
 		case CABarline::Single:
 			_width = (int)( BARLINE_WIDTH + SPACE_BETWEEN_BARLINES );
 			break;
 		case CABarline::Double:
 			_width = (int)( 2*(BARLINE_WIDTH + SPACE_BETWEEN_BARLINES) );
 			break;
 		case CABarline::End:
 			_width = (int)( BARLINE_WIDTH + 2*SPACE_BETWEEN_BARLINES + BOLD_BARLINE_WIDTH );
 			break;
 		case CABarline::RepeatOpen:
 			_width = (int)( BARLINE_WIDTH + 2*SPACE_BETWEEN_BARLINES + REPEAT_DOTS_WIDTH );
 			break;
 		case CABarline::RepeatClose:
 			_width = (int)( REPEAT_DOTS_WIDTH + 2*SPACE_BETWEEN_BARLINES + BARLINE_WIDTH );
 			break;
 		case CABarline::Dotted:
 			_width = (int)( DOTTED_BARLINE_WIDTH + SPACE_BETWEEN_BARLINES );
 			break;
 	}
	
	_height = staff->height();
	_neededWidth = _width + 4;
	_neededHeight = _height;
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
			pen.setWidth((int)(BARLINE_WIDTH*s.z));
			p->setPen(pen);
			p->drawLine((int)(s.x + 0.5), s.y,
			            (int)(s.x + 0.5), (int)(s.y + height()*s.z + 0.5));
			break;
		case CABarline::Double:
			pen.setWidth((int)(BARLINE_WIDTH*s.z));
			p->setPen(pen);
			p->drawLine((int)(s.x + 0.5), s.y,
			            (int)(s.x + 0.5), (int)(s.y + height()*s.z + 0.5));
			p->drawLine((int)(s.x + BARLINE_WIDTH*s.z + SPACE_BETWEEN_BARLINES*s.z + 0.5), s.y,
			            (int)(s.x + BARLINE_WIDTH*s.z + SPACE_BETWEEN_BARLINES*s.z + 0.5), (int)(s.y + height()*s.z + 0.5));
			break;
		case CABarline::End:
			// draw thin barline
			pen.setWidth((int)(BARLINE_WIDTH*s.z));
			p->setPen(pen);
			p->drawLine((int)(s.x + 0.5), s.y,
			            (int)(s.x + 0.5), (int)(s.y + height()*s.z + 0.5));
			// draw bold barline
			pen.setWidth((int)(BOLD_BARLINE_WIDTH*s.z));
			p->setPen(pen);
			p->drawLine((int)(s.x + BARLINE_WIDTH*s.z + SPACE_BETWEEN_BARLINES*s.z + 0.5), s.y,
			            (int)(s.x + BARLINE_WIDTH*s.z + SPACE_BETWEEN_BARLINES*s.z + 0.5), (int)(s.y + height()*s.z + 0.5));
			break;
		case CABarline::RepeatOpen:
			pen.setWidth((int)(BARLINE_WIDTH*s.z));
			p->setPen(pen);
			// draw barline
			p->drawLine( (int)(s.x + 0.5), s.y,
			             (int)(s.x + 0.5), (int)(s.y + height()*s.z + 0.5));
			// draw upper dot
			p->drawEllipse( (int)(s.x + BARLINE_WIDTH*s.z + SPACE_BETWEEN_BARLINES*s.z + 0.5),
			                (int)(s.y + 4*SPACE_BETWEEN_BARLINES*s.z + 0.5),
			                (int)(REPEAT_DOTS_WIDTH*s.z), (int)(REPEAT_DOTS_WIDTH*s.z) );
			// draw lower dot
			p->drawEllipse( (int)(s.x + BARLINE_WIDTH*s.z + SPACE_BETWEEN_BARLINES*s.z + 0.5),
			                (int)(s.y + (height()-REPEAT_DOTS_WIDTH-4*SPACE_BETWEEN_BARLINES)*s.z + 0.5),
			                (int)(REPEAT_DOTS_WIDTH*s.z), (int)(REPEAT_DOTS_WIDTH*s.z) );
			break;
		case CABarline::RepeatClose:
			pen.setWidth((int)(BARLINE_WIDTH*s.z));
			p->setPen(pen);
			// draw upper dot
			p->drawEllipse( (int)(s.x + 0.5),
			                (int)(s.y + 4*SPACE_BETWEEN_BARLINES*s.z + 0.5),
			                (int)(REPEAT_DOTS_WIDTH*s.z), (int)(REPEAT_DOTS_WIDTH*s.z) );
			// draw lower dot
			p->drawEllipse( (int)(s.x + 0.5),
			                (int)(s.y + (height()-REPEAT_DOTS_WIDTH-4*SPACE_BETWEEN_BARLINES)*s.z + 0.5),
			                (int)(REPEAT_DOTS_WIDTH*s.z), (int)(REPEAT_DOTS_WIDTH*s.z) );
			// draw barline
			p->drawLine( (int)(s.x + REPEAT_DOTS_WIDTH*s.z + SPACE_BETWEEN_BARLINES*s.z + 0.5), s.y,
			             (int)(s.x + REPEAT_DOTS_WIDTH*s.z + SPACE_BETWEEN_BARLINES*s.z + 0.5), (int)(s.y + height()*s.z + 0.5));
			break;
		case CABarline::Dotted:
			pen.setStyle( Qt::DotLine );
			pen.setCapStyle(Qt::RoundCap);
			pen.setWidth((int)(DOTTED_BARLINE_WIDTH*s.z));
			p->setPen(pen);
			p->drawLine( (int)(s.x + 0.5), s.y,
			             (int)(s.x + 0.5), (int)(s.y + height()*s.z + 0.5) );
			break;		
	}
}

CADrawableBarline* CADrawableBarline::clone(CADrawableContext* newContext) {
	return new CADrawableBarline((CABarline*)_musElement, (CADrawableStaff*)((newContext)?newContext:_drawableContext), _xPos, _yPos);
}
