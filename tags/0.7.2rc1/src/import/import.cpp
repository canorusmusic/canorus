/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "import/import.h"
#include <QTextStream>

/*!
	\class CAImport
	\brief Base class for import filters
	
	This class inherits CAFile and is the base class for any specific import filter (eg. LilyPond,
	CanorusML, MusicXML etc.).
	
	If a developer wants to write a new import filter, he should:
	1) Create a new class with the base class CAImport
	2) Implement CAImport constructors and at least importDocumentImpl() function which returns
	   the new CADocument.
	3) Register the filter (put a new fileformat to CAFileFormats and add the filter to open/save
	   dialogs in CACanorus)
	
	Optionally:
	Developer should change the current status and progress while operations are in progress. He should
	also rewrite the readableStatus() function.
	
	The following example illustrates the usage of import class:
	\code
	  CAMyImportFilter import();
	  import.setStreamFromFile("jingle bells.xml");
	  import.importDocument();
	  import.wait();
	  
	  setDocument( import.importedDocument() );
	  CACanorus::rebuildUI();
	\endcode
	
	In Python the example is even more direct using the string as an input method:
	\code
	  lilyString = '\\relative c { \\clef "treble" \\time 4/4 c4 d e f | f e d c | c1 \\bar "|." }'
	  myImport = CALilyPondImport( lilyString )
	  myImport.importVoice()
	  myImport.wait()	  
	  
	  voice = myImport.importedVoice()
	\endcode
	
	\note Both stream and string can be used both in Canorus and scripting. The example is only for illustration.
*/

CAImport::CAImport( QTextStream *stream )
 : CAFile() {
	setStream( stream );
	setImportPart( Undefined );
	
	setImportedDocument( 0 );
	setImportedSheet( 0 );
	setImportedStaff( 0 );
	setImportedVoice( 0 );
	setImportedLyricsContext( 0 );
	setImportedFunctionMarkContext( 0 );
	_fileName.clear();
}

CAImport::CAImport( const QString stream )
 : CAFile() {
	setStream( new QTextStream( new QString(stream)) );
	setImportPart( Undefined );
	
	setImportedDocument( 0 );
	setImportedSheet( 0 );
	setImportedStaff( 0 );
	setImportedVoice( 0 );
	setImportedLyricsContext( 0 );
	setImportedFunctionMarkContext( 0 );
}

CAImport::~CAImport() {
	if ( stream() && stream()->string() )
	{
		delete stream()->string();
	}
}

/*!
	Extends CAFile::setStreamFromFile by storing the filename in a public variable
	for use in the pmidi midi file parser.
*/
void CAImport::setStreamFromFile( const QString filename ) {
	_fileName = filename;
	CAFile::setStreamFromFile( filename );
}

QString CAImport::fileName() {
	return _fileName;
}


/*!
	Executed when a new thread is dispatched.
	It looks which part of the document should be imported and starts the procedure.
	It emits the appropriate signal when the procedure is finished.
*/
void CAImport::run() {
	if ( !stream() ) {
		setStatus(-1);
	} else {
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
		case FunctionMarkContext: {
			CAFunctionMarkContext *fmc = importFunctionMarkContextImpl();
			setImportedFunctionMarkContext( fmc );
			emit functionMarkContextImported( fmc );
			break;
		}
		case Undefined:
			break;
		}
		
		if (status()>0) { // error - bad implemented filter
			              // job is finished but status is still marked as working, set to Ready to prevent infinite loops
			setStatus(0);
		}
	}
	
	emit importDone( status() );
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

void CAImport::importFunctionMarkContext() {
	setImportPart( FunctionMarkContext );
	setStatus( 1 ); // process started
	start();
}

const QString CAImport::readableStatus() {
	switch (status()) {
	case 1:
		return tr("Importing");
	case 0:
		return tr("Ready");
	case -1:
		return tr("Unable to open file for reading");
	}
	return "Ready";
}
