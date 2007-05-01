/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/lyricscontext.h"
#include "core/syllable.h"

/*!
	\class CALyricsContext
	\brief One stanza line of lyrics
	
	This class represents a single stanza of the lyrics. It consists of various syllables (text under every note)
	sorted by their timeStarts.
	
	Every LyricsContext has its associated voice. This is the voice which the syllables are assigned to (one syllable per chord).
	Assocciated voice is a common LilyPond syntax \lyricsto.
	
	If the user wants to create multiple stanzas, it should create multiple lyrics contexts - one for each stanza.
	
	\sa _syllableMap, CASyllable
*/

CALyricsContext::CALyricsContext(int stanzaNumber, CAVoice *v, CASheet *s, const QString name)
 : CAContext(s, name) {
	setContextType( LyricsContext );
	
	setAssociatedVoice( v );
	setStanzaNumber(stanzaNumber);
}

CALyricsContext::~CALyricsContext() {
}

void CALyricsContext::clear() {
}

CAMusElement* CALyricsContext::findNextMusElement(CAMusElement*) {
}

CAMusElement* CALyricsContext::findPrevMusElement(CAMusElement*) {
}

/*!
	Removes the given syllable from the list.
*/
bool CALyricsContext::removeMusElement(CAMusElement* elt, bool autodelete) {
	if ( elt &&
	     elt->musElementType()==CAMusElement::Syllable &&
	     _syllableMap.contains(elt->timeStart())
	    ) {
		_syllableMap.remove(elt->timeStart());
		if (autodelete)
			delete elt;
		return true;
	} else {
		return false;
	} 
}

/*!
	Removes the syllable at the given \a timeStart and updates the timeStarts for syllables after it.
	Also deletes the object itself, if \a autoDelete is set.
	This function is usually called when removing the note.
	
	Returns True if the syllable was found and removed; False otherwise.
*/
bool CALyricsContext::removeSyllableAtTimeStart( int timeStart, bool autoDelete ) {
	if ( _syllableMap.contains(timeStart) ) {
		CASyllable *syllable = _syllableMap[timeStart];
		_syllableMap.remove(timeStart);
		
		// update times
		QList<int> timeStarts = _syllableMap.keys();
		while (timeStarts.size()) {
			int minIdx=0;
			for (int i=1; i<timeStarts.size(); i++)
				if (timeStarts[i] < timeStarts[minIdx])
					minIdx = i;
			
			if (timeStarts[minIdx] > syllable->timeStart()) {
				CASyllable *curSyllable = _syllableMap[timeStarts[minIdx]];
				curSyllable->setTimeStart( curSyllable->timeStart() - syllable->timeLength() );
				_syllableMap.remove(timeStarts[minIdx]);
				_syllableMap[curSyllable->timeStart()] = curSyllable;
			}
			
			timeStarts.removeAt(minIdx);
		}
		
		if (autoDelete)
			delete syllable;
		
		return true;
	} else {
		return false;
	}
}

/*!
	Adds a syllable to the context.
	Syllables are always sorted by their startTimes (stored in QHash).
	
	\sa _syllableList
*/
bool CALyricsContext::addSyllable( CASyllable *syllable ) {
	if (syllable) {
		_syllableMap[syllable->timeStart()] = syllable;
		return true;
	} else
		return false;
}

QList<CAMusElement*> CALyricsContext::musElementList() {
	QList<CASyllable*> list = _syllableMap.values();
	QList<CAMusElement*> musEltList;
	for (int i=0; i<list.size(); i++)
		musEltList << list[i];
	
	return musEltList;
}

/*!
	\fn CALyricsContext::syllableAt( int timeStart )
	
	Returns the syllable with the given \a timeStart. Only one syllable per stanza can have this time.
*/

/*!
	\var CALyricsContext::_syllableMap
	
	Map of timeStart : syllable.
*/
