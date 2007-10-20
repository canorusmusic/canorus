/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef EXPORT_H_
#define EXPORT_H_

#include "core/file.h"

class CADocument;
class CASheet;
class CAStaff;
class CAVoice;
class CALyricsContext;
class CAFunctionMarkingContext;

class CAExport : public CAFile {
	Q_OBJECT
public:
	CAExport( QTextStream *stream=0 );
	virtual ~CAExport();
	
	virtual const QString readableStatus();
	void exportDocument( CADocument* );
	void exportSheet( CASheet* );
	void exportStaff( CAStaff* );
	void exportVoice( CAVoice* );
	void exportLyricsContext( CALyricsContext* );
	void exportFunctionMarkingContext( CAFunctionMarkingContext* );
	
	inline CADocument      *exportedDocument() { return _exportedDocument; }
	inline CASheet         *exportedSheet() { return _exportedSheet; }
	inline CAStaff         *exportedStaff() { return _exportedStaff; }
	inline CAVoice         *exportedVoice() { return _exportedVoice; }
	inline CALyricsContext *exportedLyricsContext() { return _exportedLyricsContext; }
	inline CAFunctionMarkingContext *exportedFunctionMarkingContext() { return _exportedFunctionMarkingContext; }
	
signals:
	void documentExported( CADocument* );
	void sheetExported( CASheet* );
	void staffExported( CAStaff* );
	void voiceExported( CAVoice* );
	void lyricsContextExported( CALyricsContext* );
	void functionMarkingContextExported( CAFunctionMarkingContext* );
	
	void exportDone( int status );
	
protected:
	virtual void exportDocumentImpl( CADocument* ) { setStatus(0); return; }
	virtual void exportSheetImpl( CASheet* )       { setStatus(0); return; }
	virtual void exportStaffImpl( CAStaff* )       { setStatus(0); return; }
	virtual void exportVoiceImpl( CAVoice* )       { setStatus(0); return; }
	virtual void exportLyricsContextImpl( CALyricsContext* ) { setStatus(0); return; }
	virtual void exportFunctionMarkingContextImpl( CAFunctionMarkingContext* ) { setStatus(0); return; }
	
	inline QTextStream& out() { return *stream(); }
	
	void run();
	
private:
	inline void setExportedDocument( CADocument *doc ) { _exportedDocument = doc; }
	inline void setExportedSheet( CASheet *sheet ) { _exportedSheet = sheet; }
	inline void setExportedStaff( CAStaff *staff ) { _exportedStaff = staff; }
	inline void setExportedVoice( CAVoice *voice ) { _exportedVoice = voice; }
	inline void setExportedLyricsContext( CALyricsContext *lc ) { _exportedLyricsContext = lc; }
	inline void setExportedFunctionMarkingContext( CAFunctionMarkingContext *fmc ) { _exportedFunctionMarkingContext = fmc; }
	
	CADocument      *_exportedDocument;
	CASheet         *_exportedSheet;
	CAStaff         *_exportedStaff;
	CAVoice         *_exportedVoice;
	CALyricsContext *_exportedLyricsContext;
	CAFunctionMarkingContext *_exportedFunctionMarkingContext;
	
};

#endif /* IMPORT_H_ */
