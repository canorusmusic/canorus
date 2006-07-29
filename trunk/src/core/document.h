/** @file document.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef DOCUMENT_H_
#define DOCUMENT_H_

#include <QList>
#include <QString>

class CASheet;

class CADocument {
	public:
		CADocument();
		void clear();
		CASheet *addSheet(const QString name);
		
		/**
		 * Return the number of created sheets in the document.
		 * 
		 * @return Number of sheets in the document.
		 */
		int sheetCount() { return _sheetList.size(); }
	
	private:
		QList<CASheet *> _sheetList;	///List of all the sheets in the document
		QString _title;
		QString _subTitle;
		QString _composer;
		QString _arranger;
		QString _poet;
		QString _copyright;
		QString _timestamp;
		QString _comments;
};

#endif /*DOCUMENT_H_*/
