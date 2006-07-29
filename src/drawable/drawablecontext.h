/** @file drawablecontext.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef DRAWABLECONTEXT_H_
#define DRAWABLECONTEXT_H_

#include "drawable.h"

class CAContext;

class CADrawableContext : public CADrawable {
	public:
		enum CADrawableContextType {
			DrawableStaff,
			DrawableTablature,
			DrawableLyrics,
			DrawableDynamics
		};
	
		CADrawableContext(CAContext *c, int x, int y);
		inline CAContext *context() { return _context; }
		CADrawableContextType drawableContextType() { return _drawableContextType; }
		
	protected:
		CADrawableContextType _drawableContextType;
		CAContext *_context;
};

#endif /*DRAWABLECONTEXT_H_*/
