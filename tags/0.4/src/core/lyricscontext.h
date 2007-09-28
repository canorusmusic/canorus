/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef LYRICSCONTEXT_H_
#define LYRICSCONTEXT_H_

#include "core/context.h"
#include "core/voice.h"

#include <QList>
#include <QHash>

class CASyllable;

class CALyricsContext : public CAContext {
public:
	CALyricsContext( const QString name, int stanzaNumber, CAVoice *v, CASheet *s );
	~CALyricsContext();
	void clear();
	CALyricsContext *clone( CASheet *s );
	
	void repositSyllables();
	
	CAMusElement* findNextMusElement(CAMusElement*);
	CAMusElement* findPrevMusElement(CAMusElement*);
	QList<CAMusElement*> musElementList();
	bool removeMusElement(CAMusElement*, bool);
		
	bool addSyllable( CASyllable*, bool replace=true );
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

private:
	QList< CASyllable* > _syllableList;
	CAVoice *_associatedVoice;
	int _stanzaNumber;
	QString _customStanzaName;
};

#endif /* LYRICSCONTEXT_H_ */
