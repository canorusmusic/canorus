/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef MUSICXMLEXPORT_H_
#define MUSICXMLEXPORT_H_

#include "export/export.h"

class CAContext;
class CADocument;
class CAVoice;
class CASheet;
class CAClef;
class CAKeySignature;
class CATimeSignature;
class CANote;
class CARest;

class CAMusicXmlExport : public CAExport {
public:
	CAMusicXmlExport( QTextStream *stream=0 );
	virtual ~CAMusicXmlExport();

	inline CAVoice *curVoice() { return _curVoice; }
	inline CASheet *curSheet() { return _curSheet; }
	inline CADocument *curDocument() { return _curDocument; }
	inline CAContext *curContext() { return _curContext; }
	inline int curContextIndex() { return _curContextIndex; }
	
private:
	void exportSheetImpl(CASheet *s);
	void exportStaffImpl( CAStaff*, QDomElement& );
	void exportMeasure( QList<CAVoice*>&, int*, QDomElement& );
	
	void exportClef(CAClef*, QDomElement&);
	void exportTimeSig(CATimeSignature*, QDomElement&);
	void exportKeySig(CAKeySignature*, QDomElement&);
	void exportNote(CANote*, QDomElement&);
	void exportRest(CARest*, QDomElement&);
	
	inline void setCurVoice(CAVoice *voice) { _curVoice = voice; }
	inline void setCurSheet(CASheet *sheet) { _curSheet = sheet; }
	inline void setCurContext(CAContext *context) { _curContext = context; }
	inline void setCurContextIndex(int c) { _curContextIndex = c; }
	inline void setCurDocument(CADocument *document) { _curDocument = document; }
	
	CAVoice *_curVoice;
	CASheet *_curSheet;
	CAContext *_curContext;
	CADocument *_curDocument;
	int _curContextIndex;
	
	QDomDocument *_xmlDoc;
};

#endif /* MUSICXMLEXPORT_H_ */
