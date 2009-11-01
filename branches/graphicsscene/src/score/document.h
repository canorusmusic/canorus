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

class CASheet;
class CAArchive;
class CAResource;

class CADocument {
public:
	CADocument();
	virtual ~CADocument();
	CADocument *clone();
	void clear();

	const QList<CASheet*>& sheetList() { return _sheetList; }
	CASheet *addSheetByName(const QString name);
	inline void addSheet(CASheet *sheet) { _sheetList << sheet; }
	CASheet *addSheet();
	inline void removeSheet(CASheet *sheet) { _sheetList.removeAll(sheet); }
	CASheet *findSheet(const QString name);

	const QList<CAResource*>& resourceList() { return _resourceList; }
	inline void addResource(CAResource *r) { _resourceList << r; }
	inline void removeResource(CAResource *r) { _resourceList.removeAll(r); }

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

	///////////////////////////////////////////////////////
	// Temporary properties (not stored inside the file) //
	///////////////////////////////////////////////////////
	const QString fileName() { return _fileName; }
	bool isModified() { return _modified; }
	CAArchive *archive() { return _archive; }

	void setFileName(const QString fileName) { _fileName = fileName; } // not saved!
	void setModified( bool m ) { _modified = m; }
	void setArchive( CAArchive *a ) { _archive = a; }

private:
	QList<CASheet*>    _sheetList;
	QList<CAResource*> _resourceList;

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

	////////////////////////////////////////////////////
	// Temporary properties stored during the session //
	////////////////////////////////////////////////////
	QString _fileName;   // absolute filename of the document
	bool    _modified;   // unsaved changes
	CAArchive *_archive; // pointer to existing archive, if it exists
};
#endif /* DOCUMENT_H_ */
