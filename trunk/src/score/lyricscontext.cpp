/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/lyricscontext.h"
#include "score/syllable.h"
#include "score/voice.h"

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

CALyricsContext::CALyricsContext( const QString name, int stanzaNumber, CAVoice *v )
 : CAContext( name, (v && v->staff())? v->staff()->sheet() : 0 ) {
	setContextType( LyricsContext );

	_associatedVoice = 0;
	setAssociatedVoice( v ); // also reposits syllables
	setStanzaNumber(stanzaNumber);
}

CALyricsContext::CALyricsContext( const QString name, int stanzaNumber, CASheet *s )
 : CAContext( name, s ) {
	setContextType( LyricsContext );

	_associatedVoice = 0;
	setAssociatedVoice( 0 ); // also reposits syllables
	setStanzaNumber(stanzaNumber);
}

CALyricsContext::~CALyricsContext() {
	if (associatedVoice())
		associatedVoice()->removeLyricsContext(this);

	clear();
}

void CALyricsContext::clear() {
	while(!_syllableList.isEmpty())
		delete _syllableList.takeFirst();
	_syllableList.clear();
}

CALyricsContext *CALyricsContext::clone( CASheet *s ) {
	CALyricsContext *newLc = new CALyricsContext( name(), stanzaNumber(), s );
	newLc->cloneLyricsContextProperties( this );

	for (int i=0; i<_syllableList.size(); i++) {
		CASyllable *newSyllable = static_cast<CASyllable*>(_syllableList[i]->clone(newLc));
		newLc->addSyllable( newSyllable );
	}
	return newLc;
}

/*!
	Sets the properties of the given lyrics context to this lyrics context.
*/
void CALyricsContext::cloneLyricsContextProperties( CALyricsContext *lc ) {
	setName( lc->name() );
	setStanzaNumber( lc->stanzaNumber() );
	setSheet( lc->sheet() );
	setAssociatedVoice( lc->associatedVoice() );
}

/*!
	Keeps the content and order of the syllables, but changes startTimes and lengths according to the notes in associatedVoice.
	This function is usually called when associatedVoice is changed or the whole lyricsContext is initialized for the first time.
	If the notes and syllables aren't synchronized (too little syllables for notes) it adds empty syllables.
*/
void CALyricsContext::repositSyllables() {
	if (associatedVoice()) {
		QList<CANote*> noteList = associatedVoice()->getNoteList();
		int i,j;
		for (i=0, j=0; i<noteList.size() && j<_syllableList.size(); i++, j++) {
			if (i>0 && noteList[i-1]->timeStart()==noteList[i]->timeStart()) { // chord
				i++;
				continue;
			}
			_syllableList[j]->setTimeStart( noteList[i]->timeStart() );
			_syllableList[j]->setTimeLength( noteList[i]->timeLength() );
		}
		int firstEmpty = j;
		for (; j<_syllableList.size() && j>0; j++) { // add syllables at the end, if too much of them exist
			if ( !_syllableList[j]->text().isEmpty() )
				firstEmpty = j+1;

			_syllableList[j]->setTimeStart(_syllableList[j-1]->timeStart()+_syllableList[j-1]->timeLength());
			_syllableList[j]->setTimeLength( 256 );
		}
		for (j=firstEmpty; j<_syllableList.size() && j>0; j++) removeAt(j); // remove empty "leftover" syllables from the end

		for (; i<noteList.size(); i++) { // add empty syllables at the end, if missing
			if (i>0 && noteList[i]->timeStart()==noteList[i-1]->timeStart()) // chord
				continue;
			addEmptySyllable( noteList[i]->timeStart(), noteList[i]->timeLength() );
		}
	}
}

CAMusElement* CALyricsContext::next( CAMusElement* elt ) {
	if (elt->musElementType()!=CAMusElement::Syllable)
		return 0;

	int i = _syllableList.indexOf(static_cast<CASyllable*>(elt));
	if (i!=-1 && ++i<_syllableList.size())
		return _syllableList[i];
	else
		return 0;
}

CAMusElement* CALyricsContext::previous( CAMusElement* elt ) {
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
bool CALyricsContext::remove( CAMusElement* elt ) {
	if (!elt || elt->musElementType()!=CAMusElement::Syllable)
		return false;

	bool success=false;
	success = _syllableList.removeAll(static_cast<CASyllable*>(elt));

	if(success)
		delete elt;

	return success;
}

/*!
	Removes the syllable at the given position.
*/
void CALyricsContext::removeAt(int i)
{
	if(i>=0 && i<_syllableList.size())
		delete _syllableList.takeAt(i);
}

/*!
	Removes the syllable at the given \a timeStart and updates the timeStarts for syllables after it.
	This function is usually called when removing the note.

	Returns True if the syllable was found and removed; False otherwise.
*/
CASyllable* CALyricsContext::removeSyllableAtTimeStart( int timeStart ) {
	int i;
	for (i=0; i<_syllableList.size() && _syllableList[i]->timeStart()!=timeStart; i++);
	if (i<_syllableList.size()) {
		CASyllable *syllable = _syllableList[i];

		// update times
		for (int j=i+1; j<_syllableList.size(); j++)
			_syllableList[j]->setTimeStart( _syllableList[j]->timeStart() - syllable->timeLength() );

		removeAt(i);
		return syllable;
	} else {
		return 0;
	}
}

/*!
	Adds a syllable to the context. The syllable at that location is replaced (default) by the new one, if
	\a replace is True.
	Time starts after the inserted syllable are increased for the length of the inserted syllable.
	Syllables are always sorted by their startTimes.

	\sa _syllableList
*/
bool CALyricsContext::addSyllable( CASyllable *syllable, bool replace ) {
	int i;
	for (i=0; i<_syllableList.size() && _syllableList[i]->timeStart()<syllable->timeStart(); i++);
	if ( i!=_syllableList.size() && replace )
		removeAt(i);
	_syllableList.insert(i, syllable);
	for (i++; i<_syllableList.size(); i++)
		_syllableList[i]->setTimeStart( _syllableList[i]->timeStart() + syllable->timeLength() );

	return true;
}

/*!
	Adds an empty syllable to the context.
	This is useful when initializing the lyrics context or inserting a new note.
*/
bool CALyricsContext::addEmptySyllable( int timeStart, int timeLength ) {
	int i;
	for (i=0; i<_syllableList.size() && _syllableList[i]->timeStart()<timeStart; i++);
	_syllableList.insert(i, (new CASyllable( "", ((i>0)?(_syllableList[i-1]->hyphenStart()):(false)), ((i>0)?(_syllableList[i-1]->melismaStart()):(false)), this, timeStart, timeLength )));
	for (i++; i<_syllableList.size(); i++)
		_syllableList[i]->setTimeStart( _syllableList[i]->timeStart() + timeLength );

	return true;
}

QList<CAMusElement*> CALyricsContext::musElementList() {
	QList<CAMusElement*> musEltList;
	for (int i=0; i<_syllableList.size(); i++)
		musEltList << _syllableList[i];

	return musEltList;
}

CASyllable *CALyricsContext::syllableAtTimeStart( int timeStart ) {
	int i;
	for (i=0; i<_syllableList.size() && _syllableList[i]->timeStart()!=timeStart; i++);
	if (i<_syllableList.size())
		return _syllableList[i];
	else
		return 0;
}
