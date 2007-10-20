/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "export/export.h"
#include <QTextStream>

CAExport::CAExport( QTextStream *stream )
 : CAFile() {
	setStream( stream );
}

CAExport::~CAExport() {
}

void CAExport::run() {
	exec(); // start event loop for timers and signals
	
	setStatus( 1 ); // export started
	if (exportedDocument()) {
		exportDocumentImpl( exportedDocument() );
		emit documentExported( exportedDocument() );
	} else
	if (exportedSheet()) {
		exportSheetImpl( exportedSheet() );
		emit sheetExported( exportedSheet() );
	} else
	if (exportedStaff()) {
		exportStaffImpl( exportedStaff() );
		emit staffExported( exportedStaff() );
	} else
	if (exportedVoice()) {
		exportVoiceImpl( exportedVoice() );
		emit voiceExported( exportedVoice() );
	} else
	if (exportedLyricsContext()) {
		exportLyricsContextImpl( exportedLyricsContext() );
		emit lyricsContextExported( exportedLyricsContext() );
	} else
	if (exportedFunctionMarkingContext()) {
		exportFunctionMarkingContextImpl( exportedFunctionMarkingContext() );
		emit functionMarkingContextExported( exportedFunctionMarkingContext() );
	}
	
	emit exportDone( status() );
}

void CAExport::exportDocument( CADocument *doc ) {
	setExportedDocument( doc );
	start();
}

void CAExport::exportSheet( CASheet *sheet ) {
	setExportedSheet( sheet );
	start();
}

void CAExport::exportStaff( CAStaff *staff ) {
	setExportedStaff( staff );
	start();
}

void CAExport::exportVoice( CAVoice *voice ) {
	setExportedVoice( voice );
	start();
}

void CAExport::exportLyricsContext( CALyricsContext *lc ) {
	setExportedLyricsContext( lc );
	start();
}

void CAExport::exportFunctionMarkingContext( CAFunctionMarkingContext *fmc ) {
	setExportedFunctionMarkingContext( fmc );
	start();
}

const QString CAExport::readableStatus() {
	switch (status()) {
	case 1:
		return tr("Exporting...");
	case 0:
		return tr("Ready");
	}
}
