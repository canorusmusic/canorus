/*!
	Copyright (c) 2007, Matevž Jekovec, Itay Perl, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "export/canexport.h"
#include "export/canorusmlexport.h"
#include "core/archive.h"

#include "core/document.h"

#include <QTextStream>
#include <QTemporaryFile>

CACanExport::CACanExport( QTextStream *stream )
 : CAExport(stream) {
}

CACanExport::~CACanExport() {
}

void CACanExport::exportDocumentImpl( CADocument* doc ) {
	QTemporaryFile tmp;
	if (tmp.open()) {
		// Write the score
		CACanorusMLExport *content = new CACanorusMLExport( new QTextStream(&tmp) );
		content->exportDocument( doc );
		content->wait();
		delete content;
		tmp.close();
		
		doc->archive()->addFile( "content.xml", tmp );
		
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
	} else {
		setStatus(-1);
	}
}
