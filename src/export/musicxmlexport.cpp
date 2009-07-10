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

#include "score/document.h"
#include "score/sheet.h"
#include "score/context.h"
#include "score/staff.h"
#include "score/voice.h"
#include "score/note.h"
#include "score/rest.h"
#include "score/clef.h"
#include "score/muselement.h"
#include "score/keysignature.h"
#include "score/timesignature.h"
#include "score/barline.h"

#include "score/mark.h"
#include "score/text.h"
#include "score/tempo.h"
#include "score/bookmark.h"
#include "score/articulation.h"
#include "score/crescendo.h"
#include "score/instrumentchange.h"
#include "score/dynamic.h"
#include "score/ritardando.h"
#include "score/fermata.h"
#include "score/repeatmark.h"
#include "score/fingering.h"

#include "score/lyricscontext.h"
#include "score/syllable.h"

#include "score/functionmarkcontext.h"
#include "score/functionmark.h"

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
	
	out().setCodec("UTF-8");

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
