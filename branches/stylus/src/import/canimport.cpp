/*!
	Copyright (c) 2007, Matevž Jekovec, Itay Perl, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "import/canimport.h"
#include "import/canorusmlimport.h"
#include "core/archive.h"

#include "core/document.h"

#include <QTextStream>
#include <QTemporaryFile>

CACanImport::CACanImport( QTextStream *stream )
 : CAImport(stream) {
}

CACanImport::~CACanImport() {
}

CADocument *CACanImport::importDocumentImpl() {
	CAArchive *arc = new CAArchive(*stream()->device());
	
	if (!arc->error()) {
		// Read the score
		CAIOPtr file = arc->file("content.xml");
		CACanorusMLImport *content = new CACanorusMLImport( new QTextStream(&*file) );
		content->importDocument();
		content->wait();
		CADocument *doc = content->importedDocument();
		delete content;
		
		if (!doc) {
			setStatus(-1);
			return 0;
		}
		
		// Replace the newly created archive with the current one
		delete doc->archive();
		doc->setArchive( arc );
		
		// \todo Go through all the resources and link their QIODevices with the extracted files
		
		setStatus(0); // done
		return doc;
	} else {
		setStatus(-1);
		return 0;
	}
}
