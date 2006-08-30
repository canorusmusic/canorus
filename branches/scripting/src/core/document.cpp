/** @file document.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "context.h"
#include "staff.h"
#include "sheet.h"
#include "document.h"

CADocument::CADocument() {
	
}

void CADocument::clear() {
	_title.clear();
	_subTitle.clear();
	_composer.clear();
	_arranger.clear();
	_poet.clear();
	_copyright.clear();
	_timestamp.clear();
	_comments.clear();
	
	for (int i=0; i<_sheetList.size(); i++) {
		_sheetList[i]->clear();
		delete _sheetList[i];
	}
	_sheetList.clear();
}

CASheet *CADocument::addSheet(const QString name) {
	CASheet *s = new CASheet(name, this);
	_sheetList.append(s);
	
	return s;
}

void CADocument::addSheet(CASheet *sheet) {
	_sheetList.append(sheet);
}

CASheet *CADocument::sheet(const QString name) {
	for (int i=0; i<_sheetList.size(); i++) {
		if (_sheetList[i]->name() == name)
			return _sheetList[i];
	}
	
	return 0;
}
