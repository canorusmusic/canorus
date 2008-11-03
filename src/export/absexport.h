/*!
	Copyright (c) 2008 Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef ABSEXPORT_H_
#define ABSEXPORT_H_

class QString;
class QTextStream;
class CADocument;
class CASheet;
class CAStaff;
class CAVoice;
class CALyricsContext;
class CAFunctionMarkContext;

// Abstract interface for the CAExport base class
class CAAbsExport {

public:
	virtual ~CAAbsExport() {}

	virtual const QString readableStatus() {}
	virtual void exportDocument( CADocument*, bool bStartThread = true ) {}
	virtual void exportSheet( CASheet* ) {}
	virtual void exportStaff( CAStaff* ) {}
	virtual void exportVoice( CAVoice* ) {}
	virtual void exportLyricsContext( CALyricsContext* ) {}
	virtual void exportFunctionMarkContext( CAFunctionMarkContext* ) {}

	virtual CADocument      *exportedDocument()  = 0;
	virtual CASheet         *exportedSheet()  = 0;
	virtual CAStaff         *exportedStaff()  = 0;
	virtual CAVoice         *exportedVoice()  = 0;
	virtual CALyricsContext *exportedLyricsContext()  = 0;
	virtual CAFunctionMarkContext *exportedFunctionMarkContext() = 0;

// CAFile reimplemented for virtual table
	virtual void setStreamToFile( const QString filename ) = 0;
	virtual bool wait ( unsigned long time = ULONG_MAX ) = 0;

#ifndef SWIG
	virtual void documentExported( CADocument* ) {}
	virtual void sheetExported( CASheet* ) {}
	virtual void staffExported( CAStaff* ) {}
	virtual void voiceExported( CAVoice* ) {}
	virtual void lyricsContextExported( CALyricsContext* ) {}
	virtual void functionMarkContextExported( CAFunctionMarkContext* ) {}

	virtual void exportDone( int status ) {}
#endif

protected:
	CAAbsExport( QTextStream *stream=0 ) {}

	virtual void exportDocumentImpl( CADocument* ) = 0;
	virtual void exportSheetImpl( CASheet* )  = 0;
	virtual void exportStaffImpl( CAStaff* )  = 0;
	virtual void exportVoiceImpl( CAVoice* )  = 0;
	virtual void exportLyricsContextImpl( CALyricsContext* )  = 0;
	virtual void exportFunctionMarkContextImpl( CAFunctionMarkContext* ) = 0;

	virtual QTextStream& out() = 0;

	virtual void setExportedDocument( CADocument *doc ) = 0;
	virtual void setExportedSheet( CASheet *sheet )  = 0;
	virtual void setExportedStaff( CAStaff *staff ) = 0;
	virtual void setExportedVoice( CAVoice *voice ) = 0;
	virtual void setExportedLyricsContext( CALyricsContext *lc )  = 0;
	virtual void setExportedFunctionMarkContext( CAFunctionMarkContext *fmc )  = 0;
};

#endif /* ABSEXPORT_H_ */

