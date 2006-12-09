/** @file core/sheet.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef SHEET_H_
#define SHEET_H_

#include <QString>
#include <QList>

#include "core/context.h"

class CADocument;
class CAStaff;
class CAPlayable;

class CASheet {
	public:
		CASheet(const QString name, CADocument *doc);
		
		CAContext *contextAt(int i) { return _contextList[i]; }
		CAContext *context(const QString name);	///Look up for the context with the given name.
		void addContext(CAContext *);
		inline int contextCount() { return _contextList.size(); }
		
		CAStaff *addStaff();
		inline int staffCount() { return _staffList.size(); }
		CAStaff *staffAt(int i) { return _staffList[i]; }
		
		QList<CAPlayable*> getChord(int time);	///Return a list of notes and rests (chord) for all the voices at certain time slice

		inline CADocument *document() { return _document; }
		inline const QString name() { return _name; }
		inline void setName(const QString name) { _name = name; }

		void clear();
		
	private:
		QList<CAContext *> _contextList;	///List of all the contexts in the sheet (lyrics, staffs, tablatures, general-bas markings etc.)
		QList<CAStaff *> _staffList;	///List of all the staffs only in the sheet
		CADocument *_document;
		
		QString _name;
};

#endif /*SHEET_H_*/
