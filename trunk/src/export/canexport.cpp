/*!
	Copyright (c) 2007, Matevž Jekovec, Itay Perl, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "export/canexport.h"
#include "export/canorusmlexport.h"
#include "core/archive.h"

#include "score/resource.h"
#include "score/document.h"

#include <QTextStream>
#include <QTemporaryFile>
#include <QFile>
#include <QFileInfo>

CACanExport::CACanExport( QTextStream *stream )
 : CAExport(stream) {
}

CACanExport::~CACanExport() {
}

void CACanExport::exportDocumentImpl( CADocument* doc ) {
	// Write the score
	QBuffer score;
	CACanorusMLExport *content = new CACanorusMLExport();
	content->setStreamToDevice(&score);
	content->exportDocument( doc );
	content->wait();
	delete content;

	QString fileName = "content.xml";
	if(!doc->archive()->addFile( fileName, score )) {
		setStatus(-2);
		return;
	}

	for (int i=0; i<doc->resourceList().size(); i++) {
		CAResource *r = doc->resourceList()[i];
		if (!r->isLinked()) {
			// /tmp/qt_tempXXXXX -> qt_tempXXXXX
			QFile target(r->url().toLocalFile());
			doc->archive()->addFile( fileName + " files/"+QFileInfo(target).fileName(), target );
		}
	}

	// \todo fix relative paths
	for (int i=0; i<doc->resourceList().size(); i++) {
		CAResource *r = doc->resourceList()[i];
		if (r->isLinked()) {
			// fix relative paths
			continue;
		}
	}

	// Save the archive
	doc->archive()->write( *stream()->device() );
	setStatus(0); // done
}
