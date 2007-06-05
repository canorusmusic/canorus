/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/syllable.h"

/*!
	\class CASyllable
	\brief Text under the note
	
	This class represents each text part below or under every note. It doesn't neccessarily consists only of the one
	syllable in the meaning of the word, but it usually does. Syllable is part of CALyricsContext.
	
	Every syllable can finish with a hyphen (a horizontal dash line), with melisma (a horizontal
	underscore line) or without line at the end of the word.
	
	\todo Each syllable can have a custom associated voice, if set.
*/

CASyllable::CASyllable( QString text, bool hyphen, bool melisma, CALyricsContext *context, int timeStart, int timeLength, CAVoice *voice)
 : CAMusElement(context, timeStart, timeLength) {
	setMusElementType( Syllable );
	
	setText( text );
	setHyphenStart( hyphen );
	setMelismaStart( melisma );
	setAssociatedVoice( voice );
}

CASyllable::~CASyllable() {
}

/*!
	Clears the text and sets the default hyphen and melisma settings.
	This function is usually called when directly deleting the syllable - it shouldn't be actually removed, but only its
	text set to empty.
*/
void CASyllable::clear() {
	setText("");
	CASyllable *prev = static_cast<CASyllable*>(lyricsContext()->findPrevMusElement(this));
	if (prev) {
		setHyphenStart( prev->hyphenStart() );
		setMelismaStart( prev->melismaStart() );
	} else {
		setHyphenStart( false );
		setMelismaStart( false );
	}
}

CAMusElement* CASyllable::clone() {
	return new CASyllable( text(), hyphenStart(), melismaStart(), lyricsContext(), timeStart(), timeLength(), associatedVoice() );
}

int CASyllable::compare(CAMusElement* c) {
	if ( c->musElementType()==CAMusElement::Syllable )
		return 0;
	else
		return 1;
}
