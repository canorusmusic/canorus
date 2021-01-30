/*!
	Copyright (c) 2007-2020, Matevž Jekovec, Canorus development team
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
	Assocciated voice is a common LilyPond syntax \\lyricsto.

    If the user wants to create multiple stanzas, it should create multiple lyrics contexts - one for each stanza.
    \param _stanzaNumber stores the stanza number. If _stanzaNumber equals 0, no number is printed (default).

	\sa _syllableMap, CASyllable
*/

CALyricsContext::CALyricsContext(const QString name, int stanzaNumber, CAVoice* v)
    : CAContext(name, (v && v->staff()) ? v->staff()->sheet() : nullptr)
{
    setContextType(LyricsContext);

    _associatedVoice = nullptr;
    setAssociatedVoice(v); // also reposits syllables
    setStanzaNumber(stanzaNumber);
}

CALyricsContext::CALyricsContext(const QString name, int stanzaNumber, CASheet* s)
    : CAContext(name, s)
{
    setContextType(LyricsContext);

    _associatedVoice = nullptr;
    setAssociatedVoice(nullptr); // also reposits syllables
    setStanzaNumber(stanzaNumber);
}

CALyricsContext::~CALyricsContext()
{
    if (associatedVoice())
        associatedVoice()->removeLyricsContext(this);

    clear();
}

void CALyricsContext::clear()
{
    while (!_syllableList.isEmpty())
        _syllableList.takeFirst();
}

std::shared_ptr<CAContext> CALyricsContext::cloneRealContext(CASheet* s)
{
    return cloneLyricsContext(s);
}

std::shared_ptr<CALyricsContext> CALyricsContext::cloneLyricsContext(CASheet *s)
{
    std::shared_ptr<CALyricsContext> newLc = std::make_shared<CALyricsContext>(name(), stanzaNumber(), s);
    newLc->cloneLyricsContextProperties(this);

    for (int i = 0; i < _syllableList.size(); i++) {
        auto newSyllable = (_syllableList[i]->cloneSyllable(newLc.get()));
        newLc->addSyllable(newSyllable.get());
    }
    return newLc;
}

/*!
	Sets the properties of the given lyrics context to this lyrics context.
*/
void CALyricsContext::cloneLyricsContextProperties(CALyricsContext* lc)
{
    setName(lc->name());
    setStanzaNumber(lc->stanzaNumber());
    setSheet(lc->sheet());
    setAssociatedVoice(lc->associatedVoice());
}

/*!
	Keeps the content and order of the syllables, but changes startTimes and lengths according to the notes in associatedVoice.
	This function is usually called when associatedVoice is changed or the whole lyricsContext is initialized for the first time.
	If the notes and syllables aren't synchronized (too little syllables for notes) it adds empty syllables.

 	\sa CAFunctionMarkContext::repositFunctions(), CAFiguredBassContext::repositFiguredBassMarks(), CAChordNameContext::repositChordNames()
*/
void CALyricsContext::repositSyllables()
{
    if (associatedVoice()) {
        QList<CANote*> noteList = associatedVoice()->getNoteList();
        int i, j;
        for (i = 0, j = 0; i < noteList.size() && j < _syllableList.size(); i++, j++) {
            if (i > 0 && noteList[i - 1]->timeStart() == noteList[i]->timeStart()) { // chord
                i++;
                continue;
            }
            _syllableList[j]->setTimeStart(noteList[i]->timeStart());
            _syllableList[j]->setTimeLength(noteList[i]->timeLength());
        }
        int firstEmpty = j;
        for (; j < _syllableList.size() && j > 0; j++) { // add syllables at the end, if too much of them exist
            if (!_syllableList[j]->text().isEmpty())
                firstEmpty = j + 1;

            _syllableList[j]->setTimeStart(_syllableList[j - 1]->timeStart() + _syllableList[j - 1]->timeLength());
            _syllableList[j]->setTimeLength(256);
        }
        // remove empty "leftover" syllables from the end
        for (j = firstEmpty; j < _syllableList.size() && j > 0; j++) {
            _syllableList.takeAt(j);
        }

        for (; i < noteList.size(); i++) { // add empty syllables at the end, if missing
            if (i > 0 && noteList[i]->timeStart() == noteList[i - 1]->timeStart()) // chord
                continue;
            addEmptySyllable(noteList[i]->timeStart(), noteList[i]->timeLength());
        }
    }
}

CAMusElement* CALyricsContext::next(CAMusElement* elt)
{
    if (elt->musElementType() != CAMusElement::Syllable)
        return nullptr;

    int i = _syllableList.indexOf(static_cast<CASyllable*>(elt));
    if (i != -1 && ++i < _syllableList.size())
        return _syllableList[i];
    else
        return nullptr;
}

CAMusElement* CALyricsContext::previous(CAMusElement* elt)
{
    if (elt->musElementType() != CAMusElement::Syllable)
        return nullptr;

    int i = _syllableList.indexOf(static_cast<CASyllable*>(elt));
    if (i != -1 && --i > -1)
        return _syllableList[i];
    else
        return nullptr;
}

/*!
	Removes the given syllable from the list.
*/
bool CALyricsContext::remove(CAMusElement* elt)
{
    if (!elt || elt->musElementType() != CAMusElement::Syllable)
        return false;

    bool success = false;
    success = _syllableList.removeAll(static_cast<CASyllable*>(elt));

    return success;
}

/*!
	Removes the syllable at the given \a timeStart and updates the timeStarts for syllables after it.
	This function is usually called when removing the note.

	Returns True if the syllable was found and removed; False otherwise.
*/
CASyllable* CALyricsContext::removeSyllableAtTimeStart(int timeStart)
{
    int i;
    for (i = 0; i < _syllableList.size() && _syllableList[i]->timeStart() != timeStart; i++)
        ;
    if (i < _syllableList.size()) {
        CASyllable* syllable = _syllableList[i];

        // update times
        for (int j = i + 1; j < _syllableList.size(); j++)
            _syllableList[j]->setTimeStart(_syllableList[j]->timeStart() - syllable->timeLength());

        _syllableList.takeAt(i);
        return syllable;
    } else {
        return nullptr;
    }
}

/*!
	Adds a syllable to the context. The syllable at that location is replaced (default) by the new one, if
	\a replace is True.
	Time starts after the inserted syllable are increased for the length of the inserted syllable.
	Syllables are always sorted by their startTimes.

	\sa _syllableList
*/
bool CALyricsContext::addSyllable(CASyllable* syllable, bool replace)
{
    int i;
    for (i = 0; i < _syllableList.size() && _syllableList[i]->timeStart() < syllable->timeStart(); i++)
        ;
    //int s = _syllableList.size();
    if (i < _syllableList.size() && replace) {
        _syllableList.takeAt(i);
    }
    _syllableList.insert(i, syllable);
    for (i++; i < _syllableList.size(); i++)
        _syllableList[i]->setTimeStart(_syllableList[i]->timeStart() + syllable->timeLength());

    return true;
}

/*!
	Adds an empty syllable to the context.
	This function is usually called when initializing the lyrics context
	or inserting a new note.
*/
bool CALyricsContext::addEmptySyllable(int timeStart, int timeLength)
{
    int i;
    for (i = 0; i < _syllableList.size() && _syllableList[i]->timeStart() < timeStart; i++)
        ;
    auto newSyllable = std::make_shared<CASyllable>("", ((i > 0) ? (_syllableList[i - 1]->hyphenStart()) : (false)), ((i > 0) ? (_syllableList[i - 1]->melismaStart()) : (false)), this, timeStart, timeLength);
    _syllableList.insert(i, newSyllable.get());
    for (i++; i < _syllableList.size(); i++)
        _syllableList[i]->setTimeStart(_syllableList[i]->timeStart() + timeLength);

    return true;
}

/*!
	Finds the syllable with exactly the given \a timeStart or Null, if such a
	syllables doesn't exist.
 */
CASyllable* CALyricsContext::syllableAtTimeStart(int timeStart)
{
    int i;
    for (i = 0; i < _syllableList.size() && _syllableList[i]->timeStart() != timeStart; i++)
        ;
    if (i < _syllableList.size())
        return _syllableList[i];
    else
        return nullptr;
}

/*!
	Sets a new associated voice and repositiones the syllables.
 */
void CALyricsContext::setAssociatedVoice(CAVoice* v)
{
    if (_associatedVoice)
        _associatedVoice->removeLyricsContext(this);

    if (v)
        v->addLyricsContext(this);

    _associatedVoice = v;
    repositSyllables();
}
