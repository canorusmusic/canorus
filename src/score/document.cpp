/*!
	Copyright (c) 2006-2020, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/document.h"
#include "control/resourcectl.h"
#include "core/archive.h"
#include "score/context.h"
#include "score/resource.h"
#include "score/sheet.h"
#include "score/staff.h"

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
{
    setDateCreated(QDateTime::currentDateTime());
    setDateLastModified(QDateTime::currentDateTime());
    setTimeEdited(0);
    auto newArchive = std::make_shared<CAArchive>();
    setArchive(newArchive.get());
    setModified(false);
}

/*!
	Clones this document and all its sheets and returns a pointer to its clone.
*/
CADocument* CADocument::clone()
{
    auto newDocument = std::make_shared<CADocument>();

    // set properties
    newDocument->setTitle(title());
    newDocument->setSubtitle(subtitle());
    newDocument->setComposer(composer());
    newDocument->setArranger(arranger());
    newDocument->setPoet(poet());
    newDocument->setCopyright(copyright());
    newDocument->setDateCreated(dateCreated());
    newDocument->setDateLastModified(dateLastModified());
    newDocument->setTimeEdited(timeEdited());
    newDocument->setComments(comments());
    newDocument->setFileName(fileName());

    for (int i = 0; i < sheetList().size(); i++) {
        CASheet* newSheet = sheetList()[i]->cloneSheet(newDocument.get());
        newDocument->addSheet(newSheet);
    }

    for (int i = 0; i < resourceList().size(); i++) {
        newDocument->addResource(resourceList()[i]);
    }

    return newDocument.get();
}

/*!
	Clears and destroys the document.

	\sa clear()
*/
CADocument::~CADocument()
{
    clear();
}

/*!
	Clears the document of any sheets and destroys them.
*/
void CADocument::clear()
{
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

    for (int i = 0; i < _sheetList.size(); i++) {
        _sheetList[i]->clear();
    }
    _sheetList.clear();

    while (_resourceList.size()) {
        CAResourceCtl::deleteResource(_resourceList[0]);
    }
}

/*!
	Creates a new sheet with the given \a name and
	adds it to the sheets list.

	\sa addSheet(CASheet *sheet), sheet(), sheetAt(), _sheetList
*/
CASheet* CADocument::addSheetByName(const QString name)
{
    auto s = std::make_shared<CASheet>(name, this);
    _sheetList << s.get();
    _sheetListShared << s;

    return s.get();
}

/*!
	Adds and empty sheet to the document.
 */
CASheet* CADocument::addSheet()
{
    auto s = std::make_shared<CASheet>(QObject::tr("Sheet%1").arg(sheetList().size() + 1), this);
    addSheet(s.get());
    addSheetShared(s);

    return s.get();
}

/*!
	Returns the first sheet with the given \a name.
*/
CASheet* CADocument::findSheet(const QString name)
{
    for (int i = 0; i < _sheetList.size(); i++) {
        if (_sheetList[i]->name() == name)
            return _sheetList[i];
    }

    return nullptr;
}

/*!
	Adds sheet to the document as shared pointer.
 */
void CADocument::addSheetShared(std::shared_ptr<CASheet> sheet)
{
    _sheetListShared << sheet;
}
