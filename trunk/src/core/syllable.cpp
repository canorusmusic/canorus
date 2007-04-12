/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/syllable.h"

CASyllable::CASyllable( QString text, int stanzaNumber, bool hyphen, bool melisma, CALyricsContext *context, int timeStart, int timeLength, CAVoice *voice)
 : CAMusElement(context, timeStart, timeLength) {
	setMusElementType( Syllable );
	
	setText( text );
	setStanzaNumber( stanzaNumber );
	setHyphenStart( hyphen );
	setMelismaStart( melisma );
	setVoice( voice );
}

CASyllable::~CASyllable() {
}

CAMusElement* CASyllable::clone() {
	return new CASyllable( text(), stanzaNumber(), hyphenStart(), melismaStart(), lyricsContext(), timeStart(), timeLength(), voice() );
}

int CASyllable::compare(CAMusElement* c) {
	if ( c->musElementType()==CAMusElement::Syllable )
		return 0;
	else
		return 1;
}
