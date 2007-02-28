/* 
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#include "core/document.h"
#include "core/context.h"
#include "core/staff.h"
#include "core/sheet.h"
#include "core/voice.h"

/*!
	\class CASheet
	\brief Represents a single sheet of paper in the document
	
	CASheet represents a sheet of paper for the composer. The idea was taken out from
	spreadsheet applications. Each sheet is usually in its own tab.
	
	CASheet parent is CADocument and CASheet includes various contexts CAContext, let it
	be staffs, lyrics, function markings etc.
	
	\sa CADocument, CAContext
*/

/*!
	Creats a new sheet named \a name with parent document \a doc.
*/
CASheet::CASheet(const QString name, CADocument *doc) {
	_name = name;
	_document = doc;
}

void CASheet::addContext(CAContext *c) {
	_contextList.append(c);
	if (c->contextType() == CAContext::Staff)
		_staffList.append((CAStaff*)c);
}

CAStaff *CASheet::addStaff() {
	CAStaff *s = new CAStaff(this, QString("Staff ") + QString::number(staffCount()+1));
	_contextList.append(s);
	_staffList.append(s);
	
	return s;
}

void CASheet::clear() {
	for (int i=0; i<_contextList.size(); i++) {
		_contextList[i]->clear();
		delete _contextList[i];
	}
	
	_contextList.clear();	
}

CAContext *CASheet::context(const QString name) {
	for (int i=0; i<_contextList.size(); i++)
		if (_contextList[i]->name() == name)
			return _contextList[i];
	
	return 0;
}

/*!
	Returns a list of notes and rests (chord) for all the voices in time slice \a time.
	This is useful for determination of the harmony at certain point in time.
*/
QList<CAPlayable*> CASheet::getChord(int time) {
	QList<CAPlayable*> chordList;
	for (int i=0; i<_staffList.size(); i++) {
		for (int j=0; j<_staffList[i]->voiceCount(); j++) {
			chordList << _staffList[i]->voiceAt(j)->getChord(time);
		}
	}
	
	return chordList;
}

/*!
	\fn CAContext::context(const QString name)
	Looks up for the context with the given name.
	
	\sa contextAt(), _contextList
*/

/*!
	\fn CAContext::contextAt(int i)
	Return the context with index \a i counting from 0.
	
	\sa context(), _contextList
*/

/*!
	\var CAContext::_contextList
	List of all the contexts in the sheet (lyrics, staffs, tablatures, general-bas
	markings etc.).
	
	\sa context(), contextAt(), contextCount(), _staffList
*/

/*!
	\var CAContext::_staffList
	List of all the staffs in the sheet. Staff lookups are usually much more often than
	other contexts.
	
	All the staffs are contexts and are part of _contextList as well!
	
	\sa addStaff(), staffCount(), staffAt(), _contextList
*/
