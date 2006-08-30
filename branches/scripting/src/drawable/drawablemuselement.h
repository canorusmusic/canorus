/** @file drawablemuselement.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef DRAWABLEMUSELEMENT_H_
#define DRAWABLEMUSELEMENT_H_

#include "drawable.h"

class CAMusElement;
class CADrawableContext;

class CADrawableMusElement : public CADrawable {
	public:
		CADrawableMusElement(CAMusElement *musElement, CADrawableContext *drawableContext, int x, int y);

		enum CADrawableMusElementType {
			DrawableNote,
			DrawableClef,
			DrawableKeySignature,
			DrawableTimeSignature,
			DrawableBarline,
			DrawableAccidental
		};
				
		inline CAMusElement *musElement() { return _musElement; }
		CADrawableMusElementType drawableMusElementType() { return _drawableMusElement; }
		CADrawableContext *drawableContext() { return _drawableContext; }
		
	protected:
		CADrawableMusElementType _drawableMusElement;	///CADrawableMusElement Type

		CADrawableContext *_drawableContext;
		CAMusElement *_musElement;
};

#endif /*DRAWABLEMUSELEMENT_H_*/
