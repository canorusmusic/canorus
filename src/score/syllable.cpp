/*!
	Copyright (c) 2007-2020, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/syllable.h"
#include "score/mark.h"

/*!
	\class CASyllable
	\brief Lyrics under the note

	This class represents one lyrics element above or below the note. It doesn't neccessarily consist of one
	syllable or even a word, but can contain multiple syllables and words (using an underscore _ or a dash -).
	Syllables are usually stored inside CALyricsContext.

	Every lyrics element can finish with a hyphen (a horizontal middle line), with melisma (a horizontal
	underscore line) or without line at the end of the word.

	Each syllable can have a custom associated voice, if set. Usually parent's (lyrics context) voice is taken.
*/

/*!
	Creates a new lyrics element with the given \a text, hyphenation \a hyphen and \a melisma properties, parent \a context,
	\a timeStart and \a timeLength. \a voice is a special per-syllable associated voice (default 0 - takes parent's voice).
 */
CASyllable::CASyllable(QString text, bool hyphen, bool melisma, CALyricsContext* context, int timeStart, int timeLength, CAVoice* voice)
    : CAMusElement(context, timeStart, timeLength)
{
    setMusElementType(Syllable);

    setText(text);
    setHyphenStart(hyphen);
    setMelismaStart(melisma);
    setAssociatedVoice(voice);
}

CASyllable::~CASyllable()
{
}

/*!
	Clears the text and sets the default hyphen and melisma settings.
	This function is usually called when directly deleting the syllable - it shouldn't be actually removed, but only its
	text set to empty.
*/
void CASyllable::clear()
{
    setText("");
    setHyphenStart(false);
    setMelismaStart(false);
}

/*!
	Clone the syllable using the given new context.
	If the given context is not a lyrics context, 0 is used instead.
*/
std::shared_ptr<CAMusElement> CASyllable::cloneRealElement(CAContext* context)
{
    return cloneSyllable(static_cast<CALyricsContext*>(context));
}

std::shared_ptr<CASyllable> CASyllable::cloneSyllable(CALyricsContext *context)
{
    std::shared_ptr<CALyricsContext> newContext;
    if (context->contextType() == CAContext::LyricsContext) {
        newContext = std::make_shared<CALyricsContext>(context->name(), context->stanzaNumber(), context->sheet());
    }
    std::shared_ptr<CASyllable> s = std::make_shared<CASyllable>(text(), hyphenStart(), melismaStart(), newContext.get(), timeStart(), timeLength(), associatedVoice());

    for (int i = 0; i < markList().size(); i++) {
        auto m = (markList()[i]->cloneMark(s.get()));
        s->addMark(m.get());
    }

    return s;
}

int CASyllable::compare(CAMusElement* c)
{
    if (c->musElementType() == CAMusElement::Syllable)
        return 0;
    else
        return 1;
}
