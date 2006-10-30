/** @file drawable/drawablecontext.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef DRAWABLECONTEXT_H_
#define DRAWABLECONTEXT_H_

#include <QList>

#include "drawable.h"

class CAContext;
class CADrawableMusElement;

class CADrawableContext : public CADrawable {
	public:
		enum CADrawableContextType {
			DrawableStaff,
			DrawableTablature,
			DrawableLyrics,
			DrawableDynamics,
			DrawableFunctionMarkingContext
		};
	
		CADrawableContext(CAContext *c, int x, int y);
		inline CAContext *context() { return _context; }
		CADrawableContextType drawableContextType() { return _drawableContextType; }
		void addMElement(CADrawableMusElement *elt) { _drawableMusElementList << elt; }
		int removeMElement(CADrawableMusElement *elt) { return _drawableMusElementList.removeAll(elt); }
		
	protected:
		CADrawableContextType _drawableContextType;
		CAContext *_context;
		QList<CADrawableMusElement *> _drawableMusElementList;	///List of all the drawable musElements in this staff
};

#endif /*DRAWABLECONTEXT_H_*/
