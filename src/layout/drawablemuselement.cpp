/*!
	Copyright (c) 2006-2022, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "layout/drawablemuselement.h"

const QString CADrawableMusElement::EMPTY_PLACEHOLDER = "·";

CADrawableMusElement::CADrawableMusElement(CAMusElement* m, CADrawableContext* drawableContext, double x, double y)
    : CADrawable(x, y)
{
    setDrawableType(CADrawable::DrawableMusElement);
    _musElement = m;
    _drawableContext = drawableContext;
}
