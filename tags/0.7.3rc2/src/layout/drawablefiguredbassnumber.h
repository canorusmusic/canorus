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

class CADrawableFiguredBassNumber: public CADrawableMusElement {
public:
	CADrawableFiguredBassNumber( CAFiguredBassMark *m, int number, CADrawableFiguredBassContext*, double x, double y );
	virtual ~CADrawableFiguredBassNumber();

	CADrawableFiguredBassNumber *clone(CADrawableContext *c);
	void draw(QPainter *p, const CADrawSettings s);

	CAFiguredBassMark *figuredBassMark() { return static_cast<CAFiguredBassMark*>(musElement()); }
	int number() { return _number; }

	static const double DEFAULT_NUMBER_SIZE;
private:
	int _number;
};

#endif /* DRAWABLEFIGUREDBASSMARK_H_ */
