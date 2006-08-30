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
		
		/**
		 * Return the number of created sheets in the document.
		 * 
		 * @return Number of sheets in the document.
		 */
		int sheetCount() { return _sheetList.size(); }
		
		CASheet *sheet(const QString name);	///Look up for the sheet with the given name
		CASheet *sheetAt(int i) { return _sheetList[i]; }
		
		CASheet *addSheet(const QString name);
		void addSheet(CASheet *sheet);

		const QString title() { return _title; } 
		const QString subTitle() { return _subTitle; } 
		const QString composer() { return _composer; } 
		const QString arranger() { return _arranger; } 
		const QString poet() { return _poet; } 
		const QString textTranslator() { return _textTranslator; } 
		const QString dedication() { return _dedication; } 
		const QString copyright() { return _copyright; } 
		const QString timestamp() { return _timestamp; } 
		const QString comments() { return _comments; } 
		
		void setTitle(const QString title) { _title = title; }
		void setSubTitle(const QString subTitle) { _subTitle = subTitle; }
		void setComposer(const QString composer) { _composer = composer; }
		void setArranger(const QString arranger) { _arranger = arranger; }
		void setPoet(const QString poet) { _poet = poet; }
		void setTextTranslator(const QString textTranslator) { _textTranslator = textTranslator; }
		void setDedication(const QString dedication) { _dedication = dedication; }
		void setCopyright(const QString copyright) { _copyright = copyright; }
		void setTimestamp(const QString timestamp) { _timestamp = timestamp; }
		void setComments(const QString comments) { _comments = comments; }
		
	private:
		QList<CASheet *> _sheetList;	///List of all the sheets in the document
		
		QString _title;
		QString _subTitle;
		QString _composer;
		QString _arranger;
		QString _poet;
		QString _textTranslator;
		QString _dedication;
		QString _copyright;
		QString _timestamp;
		QString _comments;
};

#endif /*DOCUMENT_H_*/
