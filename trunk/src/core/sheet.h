/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef SHEET_H_
#define SHEET_H_

#include <QString>
#include <QList>

#include "core/context.h"
#include "core/staff.h"
#include "core/resourcecontainer.h"

class CADocument;
class CAPlayable;

class CASheet {
public:
	CASheet( const QString name, CADocument *doc );
	CASheet *clone( CADocument *doc );
	inline CASheet *clone() { return clone( document() ); }

	CAContext *contextAt(int i) { return _contextList[i]; }
	CAContext *context(const QString name);
	inline QList<CAContext*>& contextList() { return _contextList; }
	inline int contextIndex(CAContext* context) { return _contextList.indexOf(context); }
	void insertContextAfter( CAContext *after, CAContext *c);
	void addContext(CAContext *);
	inline void removeContext(CAContext* c) { _contextList.removeAll(c); }
	inline int contextCount() { return _contextList.size(); }

	CAStaff *addStaff();
	int staffCount();
	CAStaff *staffAt(int i);
	QList<CAStaff*> staffList();

	QList<CAVoice*> voiceList();
	inline CAVoice *voiceAt(int i) { return voiceList().at(i); }
	inline int voiceCount() { return voiceList().size(); }

	QList<CAPlayable*> getChord(int time);

	inline CADocument *document() { return _document; }
	inline void setDocument(CADocument *doc) { _document = doc; }

	inline const QString name() { return _name; }
	inline void setName(const QString name) { _name = name; }

	void clear();

private:
	QList<CAContext *> _contextList;
	CADocument *_document;

	QString _name;
};
#endif /*SHEET_H_*/
