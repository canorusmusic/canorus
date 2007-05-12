/* 
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#ifndef SHEET_H_
#define SHEET_H_

#include <QString>
#include <QList>

#include "core/context.h"
#include "core/staff.h"

class CADocument;
class CAPlayable;

class CASheet {
public:
	CASheet(const QString name, CADocument *doc);
	
	CAContext *contextAt(int i) { return _contextList[i]; }
	CAContext *context(const QString name);
	inline QList<CAContext*> contextList() { return _contextList; }
	void insertContextAfter( CAContext *after, CAContext *c);
	void addContext(CAContext *);
	inline void removeContext(CAContext* c) { _contextList.removeAll(c); _staffList.removeAll(static_cast<CAStaff*>(c)); }
	inline int contextCount() { return _contextList.size(); }
	
	CAStaff *addStaff();
	inline int staffCount() { return _staffList.size(); }
	inline CAStaff *staffAt(int i) { return _staffList[i]; }
	inline QList<CAStaff*> staffList() { return _staffList; }
	
	QList<CAVoice*> voiceList();
	QList<CAPlayable*> getChord(int time);	// Returns a list of notes and rests (chord) for all the voices in certain time slice
	
	inline CADocument *document() { return _document; }
	inline void setDocument(CADocument *doc) { _document = doc; }
	
	inline const QString name() { return _name; }
	inline void setName(const QString name) { _name = name; }
	
	void clear();
	
private:
	QList<CAContext *> _contextList;
	QList<CAStaff *> _staffList; // Only list of staffs
	CADocument *_document;
	
	QString _name;
};
#endif /*SHEET_H_*/