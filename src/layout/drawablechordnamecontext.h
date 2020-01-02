/*!
	Copyright (c) 2019, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef DRAWABLECHORDNAMECONTEXT_H_
#define DRAWABLECHORDNAMECONTEXT_H_

#include "layout/drawablecontext.h"
#include "score/chordnamecontext.h"

class CAChordNameContext;

class CADrawableChordNameContext : public CADrawableContext {
public:
	CADrawableChordNameContext(CAChordNameContext *c, double x, double y);
	~CADrawableChordNameContext();

	CADrawableChordNameContext *clone();
	void draw(QPainter *p, const CADrawSettings s);

	CAChordNameContext *chordNameContext() { return static_cast<CAChordNameContext*>(context()); }

	static const double DEFAULT_CHORDNAME_VERTICAL_SPACING;
};

#endif /* DRAWABLECHORDNAMECONTEXT_H_ */
