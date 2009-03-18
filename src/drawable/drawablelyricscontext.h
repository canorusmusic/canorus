/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef DRAWABLELYRICSCONTEXT_H_
#define DRAWABLELYRICSCONTEXT_H_

#include "drawable/drawablecontext.h"
#include "core/lyricscontext.h"

class CALyricsContext;

class CADrawableLyricsContext : public CADrawableContext {
public:
	CADrawableLyricsContext(CALyricsContext *c, double x, double y);
	~CADrawableLyricsContext();

	CADrawableLyricsContext* clone();
	void draw(QPainter *p, const CADrawSettings s);

	CALyricsContext *lyricsContext() { return static_cast<CALyricsContext*>(context()); }

	static const double DEFAULT_TEXT_VERTICAL_SPACING;
};

#endif /* DRAWABLELYRICSCONTEXT_H_ */
