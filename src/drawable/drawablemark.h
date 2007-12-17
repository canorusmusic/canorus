/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "drawable/drawablemuselement.h"
#include "core/mark.h"

#ifndef DRAWABLEMARK_H_
#define DRAWABLEMARK_H_

class CADrawableStaff;

class CADrawableMark : public CADrawableMusElement {
public:
	CADrawableMark( CAMark *mark, CADrawableContext *drawableContext, int x, int y);
	
	void draw( QPainter *p, CADrawSettings s );
	CADrawableMark *clone( CADrawableContext* newContext = 0 );
	inline CAMark *mark() { return static_cast<CAMark*>(musElement()); }
	
private:
	static const int DEFAULT_TEXT_SIZE;
};

#endif /* DRAWABLEMARK_H_ */
