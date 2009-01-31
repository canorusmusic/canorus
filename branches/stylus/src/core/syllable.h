/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef SYLLABLE_H_
#define SYLLABLE_H_

#include "core/muselement.h"
#include "core/lyricscontext.h"

#include <QString>

class CAVoice;
class CAContext;

class CASyllable : public CAMusElement {
public:
	CASyllable(QString text, bool hyphen, bool melisma, CALyricsContext *context, int timeStart, int timeLength, CAVoice *voice=0);
	~CASyllable();
	void clear();
	
	inline bool hyphenStart() { return _hyphenStart; }
	inline void setHyphenStart(bool h) { _hyphenStart = h; }
	inline bool melismaStart() { return _melismaStart; }
	inline void setMelismaStart(bool m) { _melismaStart = m; }
	inline QString text() { return _text; }
	inline void setText(QString text) { _text = text; }
	inline CAVoice *associatedVoice() { return _associatedVoice; }
	inline void setAssociatedVoice(CAVoice* v) { _associatedVoice = v; }
	
	inline CALyricsContext *lyricsContext() { return static_cast<CALyricsContext*>(_context); }
	
	CASyllable* clone(CAContext* context);
	int compare(CAMusElement*);
	
private:
	bool _hyphenStart, _melismaStart;
	QString _text;
	CAVoice *_associatedVoice; // associated voice, 0 if preferred
};

#endif /* SYLLABLE_H_ */
