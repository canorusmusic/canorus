/*!
	Copyright (c) 2006-2010, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef DRAWABLEBARLINE_H_
#define DRAWABLEBARLINE_H_

#include "layout/drawablemuselement.h"

class CADrawableStaff;
class CABarline;

class CADrawableBarline : public CADrawableMusElement {
	public:
		CADrawableBarline(CABarline *m, CADrawableStaff *staff);
		~CADrawableBarline();

		CADrawableBarline *clone(CADrawableContext* newContext = 0);
		inline CABarline *barline() { return (CABarline*)_musElement; }

	private:
		static const double SPACE_BETWEEN_BARLINES;

		static const double BARLINE_WIDTH;
		static const double BOLD_BARLINE_WIDTH;
		static const double REPEAT_DOTS_Y_OFFSET;
		static const double REPEAT_DOTS_WIDTH;
		static const double DOTTED_BARLINE_WIDTH;

		QList<QGraphicsLineItem*>    _lines;
		QList<QGraphicsEllipseItem*> _dots;
};

#endif /*DRAWABLEBARLINE_H_*/
