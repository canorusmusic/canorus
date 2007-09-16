/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef DRAWABLECONTEXT_H_
#define DRAWABLECONTEXT_H_

#include <QList>

#include "drawable/drawable.h"
#include "drawable/drawablemuselement.h"

class CAContext;

class CADrawableContext : public CADrawable {
public:
	enum CADrawableContextType {
		DrawableStaff,
		DrawableLyricsContext,
		DrawableFunctionMarkingContext
	};

	CADrawableContext(CAContext *c, int x, int y);
	inline CAContext *context() { return _context; }
	CADrawableContextType drawableContextType() { return _drawableContextType; }
	inline virtual void addMElement(CADrawableMusElement *elt) {
		int i;
		for (i=_drawableMusElementList.size()-1; (i>=0) && _drawableMusElementList[i]->xPos()>elt->xPos(); i--);
		_drawableMusElementList.insert( ++i, elt);
	}
	virtual int removeMElement(CADrawableMusElement *elt) { return _drawableMusElementList.removeAll(elt); }
	CADrawableMusElement *lastDrawableMusElement() { if (_drawableMusElementList.size()) return _drawableMusElementList.last(); else return 0; }
	virtual CADrawableContext* clone() = 0;
	
	inline CADrawableMusElement *findMElement(CAMusElement* elt) {
		for (int i=0; i<_drawableMusElementList.size(); i++)
			if (_drawableMusElementList[i]->musElement()==elt)
				return _drawableMusElementList[i];
	}
	
	QList<CADrawableMusElement*> findInRange( int x1, int x2 );
	
protected:
	void setDrawableContextType( CADrawableContextType type ) { _drawableContextType = type; }
	
	CADrawableContextType _drawableContextType;
	CAContext *_context;
	QList<CADrawableMusElement *> _drawableMusElementList;	// List of all the drawable musElements in this context sorted by their left borders
};

#endif /* DRAWABLECONTEXT_H_ */
