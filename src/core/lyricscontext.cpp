/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/lyricscontext.h"
#include "core/syllable.h"

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
	if ( elt &&
	     elt->musElementType()==CAMusElement::Syllable &&
	     _syllableList.contains(static_cast<CASyllable*>(elt)->stanzaNumber()) &&
	     _syllableList[static_cast<CASyllable*>(elt)->stanzaNumber()].contains(static_cast<CASyllable*>(elt))
	    ) {
		_syllableList[static_cast<CASyllable*>(elt)->stanzaNumber()].removeAll(static_cast<CASyllable*>(elt));
		if (autodelete)
			delete elt;
		return true;
	} else {
		return false;
	} 
}

/*!
	Adds a syllable to the context.
	Syllables are always sorted by their startTimes and stanzas (stored in QHash).
	
	\sa _syllableList
*/
bool CALyricsContext::addSyllable( CASyllable *syllable ) {
}

/*!
	\var CALyricsContext::_syllableList
	
	Map of stanzaNumber : list of syllables.
*/
