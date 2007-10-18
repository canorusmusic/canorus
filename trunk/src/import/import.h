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
	Q_OBJECT
public:
	CAImport( QTextStream *stream=0 );
	CAImport( QString& stream );
	virtual ~CAImport();
	
	void importDocument();
	void importSheet();
	void importStaff();
	void importVoice();
	void importLyricsContext();
	void importFunctionMarkingContext();
	
	CADocument *importedDocument();
	CASheet *importedSheet();
	CAStaff *importedStaff();
	CAVoice *importedVoice();
	CALyricsContext *importedLyricsContext();
	CAFunctionMarkingContext *importedFunctionMarking();
	
signals:
	void documentImported( CADocument* );
	void sheetImported( CASheet* );
	void staffImported( CAStaff* );
	void voiceImported( CAVoice* );
	void lyricsContextImported( CALyricsContext* );
	void functionMarkingContextImported( CAFunctionMarkingContext* );
	
protected:
	virtual CADocument      *importDocumentImpl()      { return 0; }
	virtual CASheet         *importSheetImpl()         { return 0; }
	virtual CAStaff         *importStaffImpl()         { return 0; }
	virtual CAVoice         *importVoiceImpl()         { return 0; }
	virtual CALyricsContext *importLyricsContextImpl() { return 0; }
	virtual CAFunctionMarkingContext *importFunctionMarkingContextImpl() { return 0; }
	
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
