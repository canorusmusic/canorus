/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/text.h"
#include "score/playable.h"

/*!
	\class CAText
	\brief Text sign

	Arbitrary text above or below playable elements.
	Playable elements are required since Lilypond is not able to easily assign
	arbitrary text on non-playable ones.
*/

CAText::CAText( const QString s, CAPlayable *t )
 : CAMark( CAMark::Text, t ) {
	setText( s );
}

CAText::~CAText() {
}

CAText* CAText::clone(CAMusElement* elt) {
	return new CAText( text(), (elt->isPlayable())?static_cast<CAPlayable*>(elt):0 );
}

int CAText::compare(CAMusElement *elt) {
	if (elt->musElementType()!=CAMusElement::Mark)
		return -2;

	if (static_cast<CAMark*>(elt)->markType()!=CAMark::Text)
		return -1;

	if (static_cast<CAText*>(elt)->text()!=text())
		return 1;

	return 0;
}
