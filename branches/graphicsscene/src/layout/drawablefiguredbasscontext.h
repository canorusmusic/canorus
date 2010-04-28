/*!
	Copyright (c) 2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef DRAWABLEFIGUREDBASSCONTEXT_H_
#define DRAWABLEFIGUREDBASSCONTEXT_H_

#include "layout/drawablecontext.h"
#include "score/figuredbasscontext.h"

class CAFiguredBassContext;

class CADrawableFiguredBassContext: public CADrawableContext {
public:
	CADrawableFiguredBassContext( CAFiguredBassContext *c );
	virtual ~CADrawableFiguredBassContext();

	CAFiguredBassContext *figuredBassContext() { return static_cast<CAFiguredBassContext*>(_context); }
};

#endif /* DRAWABLEFIGUREDBASSCONTEXT_H_ */
