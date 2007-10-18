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
}

CAImport::CAImport( QString& stream )
 : CAFile() {
	setStream( new QTextStream(&stream) );
}

CAImport::~CAImport() {
}

void CAImport::run() {
	exec(); // start event loop for timers and signals
	
	setStatus( 1 ); // import started
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
}

void CAImport::importDocument() {
	setImportPart( Document );
	start();
}

void CAImport::importSheet() {
	setImportPart( Sheet );
	start();
}

void CAImport::importStaff() {
	setImportPart( Staff );
	start();
}

void CAImport::importVoice() {
	setImportPart( Voice );
	start();
}

void CAImport::importLyricsContext() {
	setImportPart( LyricsContext );
	start();
}

void CAImport::importFunctionMarkingContext() {
	setImportPart( FunctionMarkingContext );
	start();
}
