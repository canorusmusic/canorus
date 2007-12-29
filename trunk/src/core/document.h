/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef DOCUMENT_H_
#define DOCUMENT_H_

#include <QList>
#include <QString>
#include <QDateTime>

#include "core/resourcecontainer.h"

class CASheet;

class CADocument : public CAResourceContainer {
public:
	CADocument();
	virtual ~CADocument();
	CADocument *clone();
	void clear();
	
	QList<CASheet*> sheetList() { return _sheetList; }
	int sheetCount() { return _sheetList.size(); }
	CASheet *sheet(const QString name);
	CASheet *sheetAt(int i) { return _sheetList[i]; }
	void setSheetAt(int i, CASheet *newSheet) { _sheetList[i]=newSheet; }
	
	CASheet *addSheetByName(const QString name);
	void addSheet(CASheet *sheet);
	inline void removeSheet(CASheet *sheet) { _sheetList.removeAll(sheet); }
	
	const QString title() { return _title; }
	const QString subtitle() { return _subtitle; }
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
	void setSubtitle(const QString subtitle) { _subtitle = subtitle; }
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
	
private:
	QList<CASheet *> _sheetList;
	
	QString _title;
	QString _subtitle;
	QString _composer;
	QString _arranger;
	QString _poet;
	QString _textTranslator;
	QString _dedication;
	QString _copyright;
	QDateTime _dateLastModified;
	QDateTime _dateCreated;
	unsigned int _timeEdited; // time the document has been edited in seconds
	QString _comments;
	QString _fileName;
};
#endif /* DOCUMENT_H_ */
