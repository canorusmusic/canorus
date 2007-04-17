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
	CALyricsContext(CAVoice *v, CASheet *s, const QString name);
	~CALyricsContext();
	void clear();
	CAMusElement* findNextMusElement(CAMusElement*);
	CAMusElement* findPrevMusElement(CAMusElement*);
	bool removeMusElement(CAMusElement*, bool);
	
	inline CAVoice *associatedVoice() { return _associatedVoice; }
	inline void setAssociatedVoice( CAVoice *v ) { _associatedVoice = v; }
	bool addSyllable( CASyllable* );
	QList<CAMusElement*> musElementList();

private:
	QHash< int, QList<CASyllable*> > _syllableMap;
	CAVoice *_associatedVoice;
};

#endif /* LYRICSCONTEXT_H_ */
