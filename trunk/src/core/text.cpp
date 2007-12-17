/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/text.h"

/*!
	\class CAText
	\brief Text sign
	
	Arbitrary text above or below the elements.
*/

CAText::CAText( const QString s, CAMusElement *t )
 : CAMark( CAMark::Text, t ) {
	setText( s );
}

CAText::~CAText() {
}
