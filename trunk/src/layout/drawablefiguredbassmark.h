/*!
	Copyright (c) 2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef DRAWABLEFIGUREDBASSMARK_H_
#define DRAWABLEFIGUREDBASSMARK_H_

#include "layout/drawablemuselement.h"
#include "score/figuredbassmark.h"

class CADrawableFiguredBassContext;

class CADrawableFiguredBassMark: public CADrawableMusElement {
public:
	CADrawableFiguredBassMark( CAFiguredBassMark *m, CADrawableFiguredBassContext*, double x, double y );
	virtual ~CADrawableFiguredBassMark();

	CADrawableFiguredBassMark *clone(CADrawableContext *c);
	void draw(QPainter *p, const CADrawSettings s);

	CAFiguredBassMark *figuredBassMark() { return static_cast<CAFiguredBassMark*>(musElement()); }

	static const double DEFAULT_NUMBER_SIZE;
};

#endif /* DRAWABLEFIGUREDBASSMARK_H_ */
