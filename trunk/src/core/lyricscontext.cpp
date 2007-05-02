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

CAMusElement* CALyricsContext::findNextMusElement(CAMusElement* elt) {
	if (elt->musElementType()!=CAMusElement::Syllable)
		return 0;
	
	int i = _syllableList.indexOf(static_cast<CASyllable*>(elt));
	if (i!=-1 && ++i<_syllableList.size())
		return _syllableList[i];
	else
		return 0;
}

CAMusElement* CALyricsContext::findPrevMusElement(CAMusElement* elt) {
	if (elt->musElementType()!=CAMusElement::Syllable)
		return 0;
	
	int i = _syllableList.indexOf(static_cast<CASyllable*>(elt));
	if (i!=-1 && --i>-1)
		return _syllableList[i];
	else
		return 0;
}

/*!
	Removes the given syllable from the list.
*/
bool CALyricsContext::removeMusElement(CAMusElement* elt, bool autodelete) {
	if (elt->musElementType()!=CAMusElement::Syllable)
		return false;
	
	
	bool success=false;
	success = _syllableList.removeAll(static_cast<CASyllable*>(elt));
	
	if (autodelete)
		delete elt;
	
	return success;
}

/*!
	Removes the syllable at the given \a timeStart and updates the timeStarts for syllables after it.
	Also deletes the object itself, if \a autoDelete is set.
	This function is usually called when removing the note.
	
	Returns True if the syllable was found and removed; False otherwise.
*/
bool CALyricsContext::removeSyllableAtTimeStart( int timeStart, bool autoDelete ) {
	int i;
	for (i=0; i<_syllableList.size() && _syllableList[i]->timeStart()!=timeStart; i++);
	if (i<_syllableList.size()) {
		CASyllable *syllable = _syllableList[i];
		_syllableList.removeAt(i);
		
		// update times
		for (; i<_syllableList.size(); i++)
			_syllableList[i]->setTimeStart( _syllableList[i]->timeStart() - syllable->timeLength() );
		
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
	int i;
	for (i=0; i<_syllableList.size() && _syllableList[i]->timeStart()<syllable->timeStart(); i++);
	if (i < _syllableList.size() && _syllableList[i]->timeStart()==syllable->timeStart()) {
		return false;
	} else {
		_syllableList.insert(i, syllable);
		return true;
	}
}

QList<CAMusElement*> CALyricsContext::musElementList() {
	QList<CAMusElement*> musEltList;
	for (int i=0; i<_syllableList.size(); i++)
		musEltList << _syllableList[i];
	
	return musEltList;
}
