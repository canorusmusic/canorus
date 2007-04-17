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
	     _syllableMap.contains(static_cast<CASyllable*>(elt)->stanzaNumber()) &&
	     _syllableMap[static_cast<CASyllable*>(elt)->stanzaNumber()].contains(static_cast<CASyllable*>(elt))
	    ) {
		_syllableMap[static_cast<CASyllable*>(elt)->stanzaNumber()].removeAll(static_cast<CASyllable*>(elt));
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
	if (_syllableMap.contains(syllable->stanzaNumber())) {
		int i;
		for (i=0; i<_syllableMap[syllable->stanzaNumber()].size() && _syllableMap[syllable->stanzaNumber()][i]->timeStart() < syllable->timeStart(); i++);
		_syllableMap[syllable->stanzaNumber()].insert(i, syllable);
	} else {
		_syllableMap[syllable->stanzaNumber()] = QList<CASyllable*>();
		_syllableMap[syllable->stanzaNumber()] << syllable;
	}
}

QList<CAMusElement*> CALyricsContext::musElementList() {
	QList< QList<CASyllable*> > list = _syllableMap.values();
	QList<CAMusElement*> retList;
	
	for (int i=0; i<list.size(); i++)
		for (int j=0; j<list[i].size(); j++)
			retList << list[i][j];
	
	return retList;
}

/*!
	\var CALyricsContext::_syllableList
	
	Map of stanzaNumber : list of syllables.
*/
