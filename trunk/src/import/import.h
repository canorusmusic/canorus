/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef IMPORT_H_
#define IMPORT_H_

#include "core/file.h"

class CADocument;
class CASheet;
class CAStaff;
class CAVoice;
class CALyricsContext;
class CAFunctionMarkingContext;

class CAImport : public CAFile {
#ifndef SWIG
	Q_OBJECT
#endif
public:
	CAImport( QTextStream *stream=0 );
	CAImport( QString& stream );
	virtual ~CAImport();
	
	virtual const QString readableStatus();
	void importDocument();
	void importSheet();
	void importStaff();
	void importVoice();
	void importLyricsContext();
	void importFunctionMarkingContext();
	
	inline CADocument *importedDocument() { return _importedDocument; }
	inline CASheet *importedSheet()       { return _importedSheet; }
	inline CAStaff *importedStaff()       { return _importedStaff; }
	inline CAVoice *importedVoice()       { return _importedVoice; }
	inline CALyricsContext *importedLyricsContext() { return _importedLyricsContext; }
	inline CAFunctionMarkingContext *importedFunctionMarkingContext() { return _importedFunctionMarkingContext; }
	
#ifndef SWIG
signals:
	void documentImported( CADocument* );
	void sheetImported( CASheet* );
	void staffImported( CAStaff* );
	void voiceImported( CAVoice* );
	void lyricsContextImported( CALyricsContext* );
	void functionMarkingContextImported( CAFunctionMarkingContext* );
	
	void importDone( int status );
#endif
	
protected:
	virtual CADocument      *importDocumentImpl()      { setStatus(0); return 0; }
	virtual CASheet         *importSheetImpl()         { setStatus(0); return 0; }
	virtual CAStaff         *importStaffImpl()         { setStatus(0); return 0; }
	virtual CAVoice         *importVoiceImpl()         { setStatus(0); return 0; }
	virtual CALyricsContext *importLyricsContextImpl() { setStatus(0); return 0; }
	virtual CAFunctionMarkingContext *importFunctionMarkingContextImpl() { setStatus(0); return 0; }
	
	QTextStream& in() { return *stream(); }
	
private:
	inline void setImportedDocument( CADocument *doc ) { _importedDocument = doc; }
	inline void setImportedSheet( CASheet *sheet ) { _importedSheet = sheet; }
	inline void setImportedStaff( CAStaff *staff ) { _importedStaff = staff; }
	inline void setImportedVoice( CAVoice *voice ) { _importedVoice = voice; }
	inline void setImportedLyricsContext( CALyricsContext *lc ) { _importedLyricsContext = lc; }
	inline void setImportedFunctionMarkingContext( CAFunctionMarkingContext *fmc ) { _importedFunctionMarkingContext = fmc; }
	
	CADocument      *_importedDocument;
	CASheet         *_importedSheet;
	CAStaff         *_importedStaff;
	CAVoice         *_importedVoice;
	CALyricsContext *_importedLyricsContext;
	CAFunctionMarkingContext *_importedFunctionMarkingContext;
	
	enum CAImportPart {
		Undefined,
		Document,
		Sheet,
		Staff,
		Voice,
		LyricsContext,
		FunctionMarkingContext
	};
	
	void run();
	inline void setImportPart( CAImportPart part ) { _importPart = part; }
	inline CAImportPart importPart() { return _importPart; }
	
	CAImportPart _importPart;
};

#endif /* IMPORT_H_ */
