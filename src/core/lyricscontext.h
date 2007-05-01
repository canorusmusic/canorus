/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef LYRICSCONTEXT_H_
#define LYRICSCONTEXT_H_

#include "core/context.h"

#include <QList>
#include <QHash>

class CASyllable;
class CAVoice;

class CALyricsContext : public CAContext {
public:
	CALyricsContext(int stanzaNumber, CAVoice *v, CASheet *s, const QString name);
	~CALyricsContext();
	void clear();
	CAMusElement* findNextMusElement(CAMusElement*);
	CAMusElement* findPrevMusElement(CAMusElement*);
	bool removeMusElement(CAMusElement*, bool);
	
	inline CAVoice *associatedVoice() { return _associatedVoice; }
	inline void setAssociatedVoice( CAVoice *v ) { _associatedVoice = v; }
	bool addSyllable( CASyllable* );
	bool removeSyllableAtTimeStart( int timeStart, bool autoDelete );
	void repositSyllables();
	QList<CAMusElement*> musElementList();
	CASyllable* syllableAt( int timeStart ) { return _syllableMap[timeStart]; }
	inline QList<CASyllable*> syllableList() { return _syllableMap.values(); }
	
	inline int stanzaNumber() { return _stanzaNumber; }
	inline void setStanzaNumber( int sn ) { _stanzaNumber = sn; }
	inline QString customStanzaName() { return _customStanzaName; }
	inline void setCustomStanzaName( QString name ) { _customStanzaName = name; }

private:
	QHash< int, CASyllable* > _syllableMap;
	CAVoice *_associatedVoice;
	int _stanzaNumber;
	QString _customStanzaName;
};

#endif /* LYRICSCONTEXT_H_ */
