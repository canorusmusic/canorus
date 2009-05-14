/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef LYRICSCONTEXT_H_
#define LYRICSCONTEXT_H_

#include "score/context.h"
#include "score/voice.h"

#include <QList>
#include <QHash>

class CASyllable;

class CALyricsContext : public CAContext {
public:
	CALyricsContext( const QString name, int stanzaNumber, CAVoice *v );
	CALyricsContext( const QString name, int stanzaNumber, CASheet *s );
	~CALyricsContext();
	CALyricsContext *clone( CASheet *s );
	void cloneLyricsContextProperties( CALyricsContext* );

	void repositSyllables();

	CAMusElement* next(CAMusElement*);
	CAMusElement* previous(CAMusElement*);
	bool remove( CAMusElement* );
	void clear();

	inline const QList<CASyllable*>& syllableList() { return _syllableList; }
	bool addSyllable( CASyllable*, bool replace=true );
	bool addEmptySyllable( int timeStart, int timeLength );
	void removeSyllable( CASyllable* s ) { _syllableList.removeAll(s); }
	CASyllable* removeSyllableAtTimeStart( int timeStart );
	CASyllable* syllableAtTimeStart( int timeStart );

	inline CAVoice *associatedVoice() { return _associatedVoice; }
	void setAssociatedVoice( CAVoice *v );
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
