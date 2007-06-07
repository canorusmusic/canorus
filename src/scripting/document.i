/** @file scripting/document.i
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

%{
#include "core/document.h"
%}

/**
 * Swig implementation of CADocument.
 */
%rename(Document) CADocument;
class CADocument {
	public:
		CADocument();
		void clear();
		
		/**
		 * Return the number of created sheets in the document.
		 * 
		 * @return Number of sheets in the document.
		 */
		int sheetCount();
		
		CASheet *sheet(const QString name);	///Look up for the sheet with the given name
		CASheet *sheetAt(int i);
		
		//TODO: The same method with different parameters is somehow conflicting with each other. These should be merged somehow.
		%rename(addSheetByName) addSheet;
		CASheet *addSheet(const QString name);
		%rename(addSheet) addSheet;
		void addSheet(CASheet *sheet);
                
		// rename all methods which return QString to qMethodName and all methods which return char* from c_strMethodName to methodName:
	const QString title() { return _title; }
	const QString subTitle() { return _subTitle; }
	const QString composer() { return _composer; }
	const QString arranger() { return _arranger; }
	const QString poet() { return _poet; }
	const QString textTranslator() { return _textTranslator; }
	const QString dedication() { return _dedication; }
	const QString copyright() { return _copyright; }
	const QDateTime dateCreated() { return _dateCreated; }
	const QDateTime dateLastModified() { return _dateLastModified; }
	const unsigned int timeEdited() { return _timeEdited; }
	const QString comments() { return _comments; }
	const QString fileName() { return _fileName; }
	
	void setTitle(const QString title) { _title = title; }
	void setSubTitle(const QString subTitle) { _subTitle = subTitle; }
	void setComposer(const QString composer) { _composer = composer; }
	void setArranger(const QString arranger) { _arranger = arranger; }
	void setPoet(const QString poet) { _poet = poet; }
	void setTextTranslator(const QString textTranslator) { _textTranslator = textTranslator; }
	void setDedication(const QString dedication) { _dedication = dedication; }
	void setCopyright(const QString copyright) { _copyright = copyright; }
	void setDateCreated(const QDateTime dateCreated) { _dateCreated = dateCreated; }
	void setDateLastModified(const QDateTime dateLastModified) { _dateLastModified = dateLastModified; }
	void setTimeEdited(const unsigned int timeEdited) { _timeEdited = timeEdited; }
	void setComments(const QString comments) { _comments = comments; }
	void setFileName(const QString fileName) { _fileName = fileName; } // not saved!
};

