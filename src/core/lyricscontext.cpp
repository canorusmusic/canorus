/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/lyricscontext.h"

CALyricsContext::CALyricsContext(CAVoice *v, CASheet *s, const QString name)
 : CAContext(s, name) {
	setContextType( LyricsContext );
	setAssociatedVoice( v );
}

CALyricsContext::~CALyricsContext() {
}

void CALyricsContext::clear() {
}

CAMusElement* CALyricsContext::findNextMusElement(CAMusElement*) {
}

CAMusElement* CALyricsContext::findPrevMusElement(CAMusElement*) {
}

bool CALyricsContext::removeMusElement(CAMusElement* elt, bool autodelete) {
	if ( _syllableList.contains(static_cast<CASyllable*>(elt)) ) {
		_syllableList.removeAll(static_cast<CASyllable*>(elt));
		if (autodelete) delete elt;
		return true;
	}
	
	return false;
}
