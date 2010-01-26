/*!
	Copyright (c) 2006-2010, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "layout/drawablemuselement.h"

CADrawableMusElement::CADrawableMusElement(CAMusElement *m, CADrawableContext *drawableContext, const CADrawableMusElementType& t)
 : CADrawable(CADrawable::DrawableMusElement),
   _musElement(m), _drawableContext(drawableContext), _drawableMusElementType(t) {
}
