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
class CANote;
class CADrawableNote;

class CADrawableMark : public CADrawableMusElement {
public:
	CADrawableMark( CAMark *mark, CADrawableContext *drawableContext, int x, int y);
	virtual ~CADrawableMark();
	
	void draw( QPainter *p, CADrawSettings s );
	CADrawableMark *clone( CADrawableContext* newContext = 0 );
	inline CAMark *mark() { return static_cast<CAMark*>(musElement()); }
	
	inline void setRehersalMarkNumber( int n ) { _rehersalMarkNumber = n; }
	inline int rehersalMarkNumber() { return _rehersalMarkNumber; }
	
private:
	static const int DEFAULT_TEXT_SIZE;
	CANote         *_tempoNote;
	CADrawableNote *_tempoDNote;
	QPixmap        *_pixmap;
	int             _rehersalMarkNumber;
};

#endif /* DRAWABLEMARK_H_ */
