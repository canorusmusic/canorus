/*!
    Copyright (c) 2007-2020, Matevž Jekovec, Itay Perl, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "import/canimport.h"
#include "core/archive.h"
#include "import/canorusmlimport.h"
#include "score/resource.h"

#include "score/document.h"

#include <QDebug>
#include <QDir>
#include <QTemporaryFile>
#include <QTextStream>

CACanImport::CACanImport(QTextStream* stream)
    : CAImport(stream)
{
}

CACanImport::~CACanImport()
{
}

CADocument* CACanImport::importDocumentImpl()
{
    auto arc = std::make_shared<CAArchive>(*stream()->device());

    if (!arc->error()) {
        // Read the score
        CAIOPtr filePtr = arc->file("content.xml");
        CACanorusMLImport* content = new CACanorusMLImport(new QTextStream(&*filePtr));
        content->importDocument();
        content->wait();
        CADocument* doc = content->importedDocument();
        delete content;

        if (!doc) {
            setStatus(-1);
            return nullptr;
        }

        // extract each resource and correct resource path
        for (int i = 0; i < doc->resourceList().size(); i++) {
            std::shared_ptr<CAResource> r = doc->resourceList()[i];
            if (!r->isLinked()) {
                // attached file - copy to /tmp
                CAIOPtr rPtr = arc->file(r->url().toLocalFile()); // chop the two leading slashes
                if (!dynamic_cast<QFile*>(&*rPtr)) {
                    qCritical() << "CACanImport: Resource \"" << r->url().toLocalFile() << "\" not found in the file.";
                    continue;
                }

                QTemporaryFile* f = new QTemporaryFile(QDir::tempPath() + "/" + r->name());
                f->open();
                QString targetFile = QFileInfo(*f).absoluteFilePath();
                f->close();
                delete f;

                static_cast<QFile*>(&*rPtr)->copy(targetFile);
                r->setUrl(QUrl::fromLocalFile(targetFile));
            } else if (r->url().scheme() == "file" && file()) {
                // linked local file - convert the relative path to absolute
                QString outDir(QFileInfo(*file()).absolutePath());
                r->setUrl(QUrl::fromLocalFile(QFileInfo(outDir + "/" + r->url().toLocalFile()).absolutePath()));
            }
        }

        // Replace the newly created archive with the current one
        // shared pointer autodelete
        doc->setArchive(arc.get());

        if (!_fileName.isEmpty()) {
            doc->setFileName(_fileName);
        }

        setStatus(0); // done
        return doc;
    } else {
        setStatus(-1);
        return nullptr;
    }
}
