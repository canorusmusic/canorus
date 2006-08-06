/** @file sheet.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "core/document.h"
#include "core/context.h"
#include "core/staff.h"
#include "core/sheet.h"

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
