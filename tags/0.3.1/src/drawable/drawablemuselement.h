/*!
	Copyright (c) 2006, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef DRAWABLEMUSELEMENT_H_
#define DRAWABLEMUSELEMENT_H_

#include "drawable/drawable.h"

class CAMusElement;
class CADrawableContext;

class CADrawableMusElement : public CADrawable {
	public:
		enum CADrawableMusElementType {
			DrawableNote,
			DrawableRest,
			DrawableClef,
			DrawableKeySignature,
			DrawableTimeSignature,
			DrawableBarline,
			DrawableAccidental,
			DrawableSlur,
			DrawableSyllable,
			DrawableFunctionMarking, DrawableFunctionMarkingSupport
		};
		
		CADrawableMusElement(CAMusElement *musElement, CADrawableContext *drawableContext, int x, int y);

		CADrawableMusElementType drawableMusElementType() { return _drawableMusElementType; }
		inline CAMusElement *musElement() { return _musElement; }
		CADrawableContext *drawableContext() { return _drawableContext; }
		void setDrawableContext(CADrawableContext *context) { _drawableContext = context; }
		virtual CADrawableMusElement* clone(CADrawableContext* newContext = 0) = 0;
		
	protected:
		void setDrawableMusElementType( CADrawableMusElementType t ) { _drawableMusElementType = t; }
		
		CADrawableMusElementType _drawableMusElementType;	// CADrawableMusElement type
		CADrawableContext *_drawableContext;
		CAMusElement *_musElement;
		bool _selectable;
};

#endif /* DRAWABLEMUSELEMENT_H_ */
