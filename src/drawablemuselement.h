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

class CADrawableMusElement : public CADrawable {
	public:
		CADrawableMusElement(CAMusElement *musElement, int x, int y);

		enum CADrawableMusElementType {
			
		};
				
		inline CAMusElement *musElement() { return _musElement; }
		CADrawableMusElementType drawableMusElementType() { return _drawableMusElement; }
		
	protected:
		CAMusElement *_musElement;
		CADrawableMusElementType _drawableMusElement;
};

#endif /*DRAWABLEMUSELEMENT_H_*/
