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

#include "core/context.h"
#include "core/clef.h"

class CASheet;
class CAContext;
class CAVoice;
class CANote;

class CAStaff : public CAContext {
public:
	CAStaff( const QString name, CASheet *s, int numberOfLines = 5);
	~CAStaff();

	inline int numberOfLines() { return _numberOfLines; }
	inline void setNumberOfLines(int val) { _numberOfLines = val; }
	void clear();
	CAStaff *clone( CASheet *s );

	inline int voiceCount() { return _voiceList.size(); }
	inline CAVoice *voiceAt(int i) { return _voiceList[i]; }
	inline int voiceIndex(CAVoice *voice) { return _voiceList.indexOf(voice); }
	inline void removeVoice(CAVoice *voice) { _voiceList.removeAll(voice); }
	inline QList<CAVoice*> voiceList() { return _voiceList; }
	CAVoice *voiceByName(const QString name);

	void     addVoice(CAVoice *voice);
	CAVoice* addVoice();

	CAMusElement *next( CAMusElement *elt );
	CAMusElement *previous( CAMusElement *elt );
	bool remove( CAMusElement *elt, bool updateSignTimes );
	bool remove( CAMusElement *elt ) { return remove(elt, true); }

	int lastTimeEnd();
	QList<CAMusElement*> getEltByType( CAMusElement::CAMusElementType type, int startTime );

	QList<CAPlayable*> getChord( int time );

	bool synchronizeVoices();

	void setName(QString name) { _name = name; }
	QString name() { return _name; }

private:
	QList<CAVoice *> _voiceList;
	QString _name;

	int _numberOfLines;
};
#endif /* STAFF_H_ */
