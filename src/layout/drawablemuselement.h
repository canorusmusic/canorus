/*!
	Copyright (c) 2006-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef DRAWABLEMUSELEMENT_H_
#define DRAWABLEMUSELEMENT_H_

#include "layout/drawable.h"

class CAMusElement;
class CADrawableContext;

class CADrawableMusElement : public CADrawable {
	enum CADrawableMusElementType {
	};
	public:
		CADrawableMusElement(CAMusElement *musElement, CADrawableContext *drawableContext, CADrawableType t, double x, double y);

		CADrawableMusElementType drawableMusElementType() { return _drawableMusElementType; }
		inline CAMusElement *musElement() { return _musElement; }
		CADrawableContext *drawableContext() { return _drawableContext; }
		void setDrawableContext(CADrawableContext *context) { _drawableContext = context; }
		virtual CADrawableMusElement* clone(CADrawableContext* newContext = 0) = 0;

	protected:
		void setDrawableMusElementType( CADrawableMusElementType t ) { _drawableMusElementType = t; }

		CADrawableMusElementType _drawableMusElementType;	// CADrawableMusElement type
		CADrawableContext *_drawableContext;
		bool _selectable;
};

#endif /* DRAWABLEMUSELEMENT_H_ */
