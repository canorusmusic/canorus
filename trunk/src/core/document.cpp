/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/context.h"
#include "core/staff.h"
#include "core/sheet.h"
#include "core/document.h"
#include "core/archive.h"

/*!
	\class CADocument
	\brief Class which represents the current document.
	
	CADocument is a top-most class in score hierarchy and represents the
	document in the current main window.
	Document consists of multiple sheets.
	
	\sa CASheet
*/

/*!
	Creates an empty document.
	
	\sa addSheet()
*/
CADocument::CADocument()
 : CAResourceContainer() {
	setDateCreated( QDateTime::currentDateTime() );
	setDateLastModified( QDateTime::currentDateTime() );
	setTimeEdited(0);
	setArchive( new CAArchive() );
}

/*!
	Clones this document and all its sheets and returns a pointer to its clone.
*/
CADocument *CADocument::clone() {
	CADocument *newDocument = new CADocument();
	
	// set properties
	newDocument->setTitle( title() );
	newDocument->setSubtitle( subtitle() );
	newDocument->setComposer( composer() );
	newDocument->setArranger( arranger() );
	newDocument->setPoet( poet() );
	newDocument->setCopyright( copyright() );
	newDocument->setDateCreated( dateCreated() );
	newDocument->setDateLastModified( dateLastModified() );
	newDocument->setTimeEdited( timeEdited() );
	newDocument->setComments( comments() );
	newDocument->setFileName( fileName() );
	
	for (int i=0; i<sheetCount(); i++)
		newDocument->addSheet( sheetAt(i)->clone() );
	
	return newDocument;
}

/*!
	Clears and destroys the document.
	
	\sa clear()
*/
CADocument::~CADocument() {
	clear();
	if ( archive() ) delete archive();
}

/*!
	Clears the document of any sheets and destroys them.
*/
void CADocument::clear() {
	_title.clear();
	_subtitle.clear();
	_composer.clear();
	_arranger.clear();
	_poet.clear();
	_copyright.clear();
	_dateCreated = QDateTime::currentDateTime();
	_dateLastModified = QDateTime::currentDateTime();
	_timeEdited = 0;
	_comments.clear();
	
	for (int i=0; i<_sheetList.size(); i++) {
		_sheetList[i]->clear();
		delete _sheetList[i];
	}
	_sheetList.clear();
}

/*!
	Creates a new sheet with the given \a name and
	adds it to the sheets list.
	
	\sa addSheet(CASheet *sheet), sheet(), sheetAt(), _sheetList
*/
CASheet *CADocument::addSheetByName(const QString name) {
	CASheet *s = new CASheet(name, this);
	_sheetList << s;
	
	return s;
}

/*!
	Adds an already created \a sheet to the document.
	Sheet's owner document is set to this document.
	
	\sa addSheet(const QString name)
*/
void CADocument::addSheet(CASheet *sheet) {
	_sheetList.append(sheet);
	sheet->setDocument(this);
}

/*!
	Look up for the sheet with the given \a name and return a
	pointer to it.
	
	\sa _sheetList
*/
CASheet *CADocument::sheet(const QString name) {
	for (int i=0; i<_sheetList.size(); i++) {
		if (_sheetList[i]->name() == name)
			return _sheetList[i];
	}
	
	return 0;
}

/*!
	\fn CADocument::sheetCount()
	Returns the number of created sheets in the document.
	
	\sa _sheetList
*/

/*!
	\fn CADocument::sheetAt(int i)
	Returns the sheet with index \a i.
	
	\sa _sheetList
*/

/*!
	\var CADocument::_sheetList
	List of documents sheets.
	
	\sa sheet(), sheetAt(), sheetCount()
*/

/*!
	\fn CADocument::fileName()
	Returns the absolute path of the file the document has been saved to or empty
	string if document hasn't been saved yet.
	Document's file name property is not saved to a file.
	
	\sa setFileName()
*/
