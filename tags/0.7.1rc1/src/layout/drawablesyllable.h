/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef DRAWABLESYLLABLE_H_
#define DRAWABLESYLLABLE_H_

#include "layout/drawablemuselement.h"
#include "score/syllable.h"

class CASyllable;
class CADrawableLyricsContext;

class CADrawableSyllable : public CADrawableMusElement {
public:
	CADrawableSyllable( CASyllable*, CADrawableLyricsContext*, double x, double y );
	~CADrawableSyllable();
	void draw(QPainter *p, const CADrawSettings s);
	CADrawableSyllable *clone(CADrawableContext *c=0);

	CASyllable *syllable() { return static_cast<CASyllable*>(musElement()); }

	static const double DEFAULT_TEXT_SIZE;
	static const double DEFAULT_DASH_LENGTH;

private:
	inline const QString textToDrawableText( QString in ) { return in.replace("_", " "); }
};

#endif /* DRAWABLESYLLABLE_H_ */
