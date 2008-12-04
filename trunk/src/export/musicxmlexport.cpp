/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QString>
#include <QDomDocument>
#include <QDomElement>
#include <QTextStream>

#include "export/musicxmlexport.h"

#include "core/document.h"
#include "core/sheet.h"
#include "core/context.h"
#include "core/staff.h"
#include "core/voice.h"
#include "core/note.h"
#include "core/rest.h"
#include "core/clef.h"
#include "core/muselement.h"
#include "core/keysignature.h"
#include "core/timesignature.h"
#include "core/barline.h"

#include "core/mark.h"
#include "core/text.h"
#include "core/tempo.h"
#include "core/bookmark.h"
#include "core/articulation.h"
#include "core/crescendo.h"
#include "core/instrumentchange.h"
#include "core/dynamic.h"
#include "core/ritardando.h"
#include "core/fermata.h"
#include "core/repeatmark.h"
#include "core/fingering.h"

#include "core/lyricscontext.h"
#include "core/syllable.h"

#include "core/functionmarkcontext.h"
#include "core/functionmark.h"

CAMusicXmlExport::CAMusicXmlExport( QTextStream *stream )
 : CAExport(stream) {
}

CAMusicXmlExport::~CAMusicXmlExport() {
}

/*!
	Exports the document to MusicXML format.
	It uses DOM object internally for writing the XML output.
 */
void CAMusicXmlExport::exportDocumentImpl(CADocument *doc) {
	int depth = 0;

	// CADocument
	QDomDocument dDoc("musicxml");

	// Add encoding
	dDoc.appendChild(dDoc.createProcessingInstruction("xml",
			"version=\"1.0\" encoding=\"UTF-8\" "));

	// Root node - <canorus-document>
	QDomElement dCanorusDocument = dDoc.createElement("musicxml-document");
	dDoc.appendChild(dCanorusDocument);

	out() << dDoc.toString();
}
