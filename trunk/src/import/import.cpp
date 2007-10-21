/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "import/import.h"
#include <QTextStream>

CAImport::CAImport( QTextStream *stream )
 : CAFile() {
	setStream( stream );
	setImportPart( Undefined );
}

CAImport::CAImport( QString& stream )
 : CAFile() {
	setStream( new QTextStream( new QString(stream)) );
	setImportPart( Undefined );
}

CAImport::~CAImport() {
	if ( stream() && stream()->string() )
		delete stream()->string();
}

void CAImport::run() {
	switch ( importPart() ) {
	case Document: {
		CADocument *doc = importDocumentImpl();
		setImportedDocument( doc );
		emit documentImported( doc );
		break;
	}
	case Sheet: {
		CASheet *sheet = importSheetImpl();
		setImportedSheet( sheet );
		emit sheetImported( sheet );
		break;
	}
	case Staff: {
		CAStaff *staff = importStaffImpl();
		setImportedStaff( staff );
		emit staffImported( staff );
		break;
	}
	case Voice: {
		CAVoice *voice = importVoiceImpl();
		setImportedVoice( voice );
		emit voiceImported( voice );
		break;
	}
	case LyricsContext: {
		CALyricsContext *lc = importLyricsContextImpl();
		setImportedLyricsContext( lc );
		emit lyricsContextImported( lc );
		break;
	}
	case FunctionMarkingContext: {
		CAFunctionMarkingContext *fmc = importFunctionMarkingContextImpl();
		setImportedFunctionMarkingContext( fmc );
		emit functionMarkingContextImported( fmc );
		break;
	}
	}
	emit importDone( status() );
	
	if (status()>0) { // error - bad implemented filter
		              // job is finished but status is still marked as working, set to Ready to prevent infinite loops
		setStatus(0);
	}
}

void CAImport::importDocument() {
	setImportPart( Document );
	setStatus( 1 ); // process started
	start();
}

void CAImport::importSheet() {
	setImportPart( Sheet );
	setStatus( 1 ); // process started
	start();
}

void CAImport::importStaff() {
	setImportPart( Staff );
	setStatus( 1 ); // process started
	start();
}

void CAImport::importVoice() {
	setImportPart( Voice );
	setStatus( 1 ); // process started
	start();
}

void CAImport::importLyricsContext() {
	setImportPart( LyricsContext );
	setStatus( 1 ); // process started
	start();
}

void CAImport::importFunctionMarkingContext() {
	setImportPart( FunctionMarkingContext );
	setStatus( 1 ); // process started
	start();
}

const QString CAImport::readableStatus() {
	switch (status()) {
	case 1:
		return tr("Importing...");
	case 0:
		return tr("Ready");
	}
}
