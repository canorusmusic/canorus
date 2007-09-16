/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPainter>

#include "drawable/drawable.h"
#include "drawable/drawablemuselement.h"
#include "drawable/drawablecontext.h"

CADrawable::CADrawable(int x, int y) {
	setXPosAbsolute( x );
	setYPosAbsolute( y );
	setXPosOffset( 0 );
	setYPosOffset( 0 );
	setVisible( true );
	setSelectable( true );
}

CADrawable* CADrawable::clone() {
	// If we reach CADrawable::clone(), then this must be a CADrawableMusElement, because otherwise it would go to the
	// CADrawableContext cloned() (this is an impure virtual function).
	return ((CADrawable*)((CADrawableMusElement*)this)->clone());
}
