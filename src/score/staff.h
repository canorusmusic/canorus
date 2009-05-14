/*!
	Copyright (c) 2006-2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef STAFF_H_
#define STAFF_H_

#include <QList>
#include <QString>

class QPainter;

#include "score/context.h"
#include "score/muselement.h"

class CASheet;
class CAContext;
class CAVoice;
class CANote;
class CATempo;

class CAStaff : public CAContext {
public:
	CAStaff( const QString name, CASheet *s, int numberOfLines = 5);
	~CAStaff();

	inline int numberOfLines() { return _numberOfLines; }
	inline void setNumberOfLines(int val) { _numberOfLines = val; }
	void clear();
	CAStaff *clone( CASheet *s );

	inline const QList<CAVoice*>& voiceList() { return _voiceList; }
	inline void addVoice(CAVoice *voice) { _voiceList << voice; }
	CAVoice* addVoice();
	inline void removeVoice(CAVoice *voice) { _voiceList.removeAll(voice); }
	CAVoice *findVoice(const QString name);

	CAMusElement *next( CAMusElement *elt );
	CAMusElement *previous( CAMusElement *elt );
	bool remove( CAMusElement *elt, bool updateSignTimes );
	bool remove( CAMusElement *elt ) { return remove(elt, true); }

	int lastTimeEnd();
	QList<CAMusElement*> getEltByType( CAMusElement::CAMusElementType type, int startTime );

	QList<CAPlayable*> getChord( int time );
	CATempo           *getTempo( int time );

	bool synchronizeVoices();

private:
	QList<CAVoice *> _voiceList;

	int _numberOfLines;
};
#endif /* STAFF_H_ */
