/*!
	Copyright (c) 2006-2019, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef DRAWABLEBARLINE_H_
#define DRAWABLEBARLINE_H_

#include "layout/drawablemuselement.h"
#include "score/barline.h"

class CADrawableStaff;

class CADrawableBarline : public CADrawableMusElement {
public:
    CADrawableBarline(CABarline* m, CADrawableStaff* staff, double x, double y);
    ~CADrawableBarline();

    void draw(QPainter* p, CADrawSettings s);
    CADrawableBarline* clone(CADrawableContext* newContext = nullptr);
    inline CABarline* barline() { return static_cast<CABarline*>(_musElement); }

private:
    static const double SPACE_BETWEEN_BARLINES;

    static const double BARLINE_WIDTH;
    static const double BOLD_BARLINE_WIDTH;
    static const double REPEAT_DOTS_WIDTH;
    static const double DOTTED_BARLINE_WIDTH;
};

#endif /*DRAWABLEBARLINE_H_*/
