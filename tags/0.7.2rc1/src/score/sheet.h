/*!
	Copyright (c) 2006-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef SHEET_H_
#define SHEET_H_

#include <QString>
#include <QList>

#include "score/context.h"
#include "score/staff.h"

class CADocument;
class CAPlayable;
class CATempo;

class CASheet {
public:
	CASheet( const QString name, CADocument *doc );
	~CASheet();
	CASheet *clone( CADocument *doc );
	inline CASheet *clone() { return clone( document() ); }

	inline const QList<CAContext*>& contextList() { return _contextList; }
	CAContext *findContext(const QString name);
	inline void insertContext( int pos, CAContext *c) { _contextList.insert( pos, c ); }
	void insertContextAfter( CAContext *after, CAContext *c );
	inline void addContext( CAContext* c ) { _contextList << c; }
	inline void removeContext( CAContext* c ) { _contextList.removeAll(c); }

	CAStaff *addStaff();
	QList<CAStaff*> staffList(); // generated list
	QList<CAVoice*> voiceList(); // generated list

	QList<CAPlayable*> getChord(int time);
	CATempo           *getTempo(int time);
	
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
