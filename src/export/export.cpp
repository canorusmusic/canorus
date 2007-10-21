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
	
	setExportedDocument(0);
	setExportedSheet(0);
	setExportedStaff(0);
	setExportedVoice(0);
	setExportedLyricsContext(0);
	setExportedFunctionMarkingContext(0);
}

CAExport::~CAExport() {
}

void CAExport::run() {
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
	
	if (status()>0) { // error - bad implemented filter
		              // job is finished but status is still marked as working, set to Ready to prevent infinite loops
		setStatus(0);
	}
}

void CAExport::exportDocument( CADocument *doc ) {
	setExportedDocument( doc );
	setStatus( 1 ); // process started
	start();
}

void CAExport::exportSheet( CASheet *sheet ) {
	setExportedSheet( sheet );
	setStatus( 1 ); // process started
	start();
}

void CAExport::exportStaff( CAStaff *staff ) {
	setExportedStaff( staff );
	setStatus( 1 ); // process started
	start();
}

void CAExport::exportVoice( CAVoice *voice ) {
	setExportedVoice( voice );
	setStatus( 1 ); // process started
	start();
}

void CAExport::exportLyricsContext( CALyricsContext *lc ) {
	setExportedLyricsContext( lc );
	setStatus( 1 ); // process started
	start();
}

void CAExport::exportFunctionMarkingContext( CAFunctionMarkingContext *fmc ) {
	setExportedFunctionMarkingContext( fmc );
	setStatus( 1 ); // process started
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
