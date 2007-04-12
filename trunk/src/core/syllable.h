/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef SYLLABLE_H_
#define SYLLABLE_H_

#include "core/muselement.h"

#include <QString>

class CALyricsContext;
class CAVoice;

class CASyllable : public CAMusElement {
public:
	CASyllable(QString text, int stanzaNumber, bool hyphen, bool melisma, CALyricsContext *context, int timeStart, int timeLength, CAVoice *voice=0);
	~CASyllable();
	
	inline bool hyphenStart() { return _hyphenStart; }
	inline void setHyphenStart(bool h) { _hyphenStart = h; }
	inline bool melismaStart() { return _melismaStart; }
	inline void setMelismaStart(bool m) { _melismaStart = m; }
	inline QString text() { return _text; }
	inline void setText(QString text) { _text = text; }
	inline CAVoice *voice() { return _voice; }
	inline void setVoice(CAVoice* v) { _voice = v; }
	inline int stanzaNumber() { return _stanzaNumber; }
	inline void setStanzaNumber(int n) { _stanzaNumber = n; }

private:
	bool _hyphenStart, _melismaStart;
	QString _text;
	int _stanzaNumber;
	CAVoice *_voice; // associated voice, 0 if preferred
};

#endif /* SYLLABLE_H_ */
