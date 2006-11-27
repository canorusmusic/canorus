/** @file drawable/drawablemuselement.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef DRAWABLEMUSELEMENT_H_
#define DRAWABLEMUSELEMENT_H_

#include "drawable/drawable.h"

class CAMusElement;
class CADrawableContext;

class CADrawableMusElement : public CADrawable {
	public:
		CADrawableMusElement(CAMusElement *musElement, CADrawableContext *drawableContext, int x, int y);

		enum CADrawableMusElementType {
			DrawableNote,
			DrawableRest,
			DrawableClef,
			DrawableKeySignature,
			DrawableTimeSignature,
			DrawableBarline,
			DrawableAccidental,
			DrawableFunctionMarking, DrawableFunctionMarkingSupport
		};
		
		inline CAMusElement *musElement() { return _musElement; }
		CADrawableMusElementType drawableMusElementType() { return _drawableMusElementType; }
		CADrawableContext *drawableContext() { return _drawableContext; }
		void setDrawableContext(CADrawableContext *context) { _drawableContext = context; }
		bool isSelectable() { return _selectable; }
		
	protected:
		CADrawableMusElementType _drawableMusElementType;	///CADrawableMusElement type

		CADrawableContext *_drawableContext;
		CAMusElement *_musElement;
		bool _selectable;
};

#endif /*DRAWABLEMUSELEMENT_H_*/
