/*!
	Copyright (c) 2008-2020, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "control/resourcectl.h"
#include "score/document.h"
#ifndef SWIGCPP
#include "canorus.h"
#include "core/undo.h"
#endif

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QUrl>
#include <iostream> // debug

/*!
	\class CAResourceCtl
	\brief Resource control class

	This class is used for creation of new resources out of files on the disk or
	web URLs.

	Resources are rarely created directly (maybe using the plugin interface), but
	usually are part of the document (eg. audio file of the whole score), sheet
	(eg. audio file of a theme), staff (eg. svg images in contemporary music),
	voice etc.
	You can simply add a resource by calling
	CADocument::addResource( new CAResource( "/home/user/title.jpeg", "My image") );

	CAResourceContainer takes care of creating copies for non-linked resources.
	It picks a random unique name for a new resource in the system temporary file.

	\sa CAResource
*/

/*!
	Default constructor. Currently empty.
*/
CAResourceCtl::CAResourceCtl()
{
}

CAResourceCtl::~CAResourceCtl()
{
}

/*!
	Imports the resource located at \a fileName and adds it to the document \a parent and all its
	undo/redo instances.
	If the resource is linked, the \a fileName should be url and a resource is only registered.
	If the resource is not linked, the \a fileName should be absolute path to the file, resource
	is copied and registered.
 */
std::shared_ptr<CAResource> CAResourceCtl::importResource(QString name, QString fileName, bool isLinked, CADocument* parent, CAResource::CAResourceType t)
{
    std::shared_ptr<CAResource> r;
    if (isLinked) {
        r = std::make_shared<CAResource>(fileName, name, true, t, parent);
    } else {
        std::unique_ptr<QTemporaryFile> f = std::make_unique<QTemporaryFile>(QDir::tempPath() + "/" + name);
        f->open();
        QString targetFile = QFileInfo(*f).absoluteFilePath();
        f->close();
        f.reset();

        if (QFile::exists(fileName)) {
            QFile::copy(fileName, targetFile);
            r = std::make_shared<CAResource>(QUrl::fromLocalFile(targetFile), name, false, t, parent);
        } else {
            // file doesn't exist yet (eg. hasn't been extracted yet)
            // create a dummy resource using fileName url
            r = std::make_shared<CAResource>(fileName, name, false, t, parent);
        }
    }

    if (parent) {
#ifndef SWIGCPP
        QList<CADocument*> documents = CACanorus::undo()->getAllDocuments(parent);

        for (int i = 0; i < documents.size(); i++) {
            documents[i]->addResource(r);
        }
#else
        parent->addResource(r);
#endif
    }

    return r;
}

/*!
	Creates an empty resource file.
	This function is usually called when launching Midi recorder.
 */
std::shared_ptr<CAResource> CAResourceCtl::createEmptyResource(QString name, CADocument* parent, CAResource::CAResourceType t)
{
    QTemporaryFile f(QDir::tempPath() + "/" + name);
    f.open();
    QString fileName = QFileInfo(f).absoluteFilePath();
    f.close();
    std::shared_ptr<CAResource> r = std::make_shared<CAResource>(QUrl::fromLocalFile(fileName), name, false, t, parent);

    if (parent) {
#ifndef SWIGCPP
        QList<CADocument*> documents = CACanorus::undo()->getAllDocuments(parent);

        for (int i = 0; i < documents.size(); i++) {
            documents[i]->addResource(r);
        }
        r->document()->setModified(true);
#else
        parent->addResource(r);
#endif
    }

    return r;
}

/*!
	Removes the given resource \a r from all the undo/redo document instances and destroys it.
 */
void CAResourceCtl::deleteResource(std::shared_ptr<CAResource> r)
{
    if (r->document()) {
#ifndef SWIGCPP
        QList<CADocument*> documents = CACanorus::undo()->getAllDocuments(r->document());

        for (int i = 0; i < documents.size(); i++) {
            documents[i]->removeResource(r);
        }
        r->document()->setModified(true);
#else
        r->document()->removeResource(r);
#endif
    }

    r.reset();
}
