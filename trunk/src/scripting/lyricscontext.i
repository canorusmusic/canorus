/*! 
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

%{
#include "core/lyricscontext.h"
%}

/**
 * Swig implementation of CALyricsContext.
 */
%rename(LyricsContext) CALyricsContext;
class CALyricsContext : public CAContext {
public:
	CALyricsContext(int stanzaNumber, CAVoice *v, CASheet *s, const QString name);
	~CALyricsContext();
	void clear();
	CALyricsContext *clone( CASheet *s );
	
	void repositSyllables();
	
	CAMusElement* findNextMusElement(CAMusElement*);
	CAMusElement* findPrevMusElement(CAMusElement*);
	QList<CAMusElement*> musElementList();
	bool removeMusElement(CAMusElement*, bool);
		
	bool addSyllable( CASyllable* );
	bool addEmptySyllable( int timeStart, int timeLength );
	CASyllable* removeSyllableAtTimeStart( int timeStart, bool autoDelete=true );
	CASyllable* syllableAt( int idx ) { return _syllableList[idx]; }
	CASyllable* syllableAtTimeStart( int timeStart );
	inline QList<CASyllable*> syllableList() { return _syllableList; }
	inline int syllableCount() { return _syllableList.size(); }
	
	inline CAVoice *associatedVoice() { return _associatedVoice; }
	inline void setAssociatedVoice( CAVoice *v ) {
		if (_associatedVoice)
			_associatedVoice->removeLyricsContext(this);
		if (v)
			v->addLyricsContext(this);
		_associatedVoice = v;
		repositSyllables();
	}
	inline int stanzaNumber() { return _stanzaNumber; }
	inline void setStanzaNumber( int sn ) { _stanzaNumber = sn; }
	inline QString customStanzaName() { return _customStanzaName; }
	inline void setCustomStanzaName( QString name ) { _customStanzaName = name; }
};
