/*!
	Copyright (c) 2007-2010, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef LILYPONDEXPORT_H_
#define LILYPONDEXPORT_H_

#include <QTextStream>
#include <QString>

#include "score/keysignature.h"
#include "score/timesignature.h"
#include "score/clef.h"
#include "score/barline.h"
#include "score/playable.h"
#include "score/note.h"
#include "score/rest.h"
#include "score/document.h"
#include "score/lyricscontext.h"
#include "score/syllable.h"

#include "export/export.h"

class CALilyPondExport : public CAExport {
public:
	CALilyPondExport( QTextStream *out=0 );

	///////////////////////////
	// Polling export status //
	///////////////////////////
	// Setter methods are private!
	inline CAVoice *curVoice() { return _curVoice; }
	inline CASheet *curSheet() { return _curSheet; }
	inline CADocument *curDocument() { return _curDocument; }
	inline CAContext *curContext() { return _curContext; }
	inline int curContextIndex() { return _curContextIndex; }
	inline int curIndentLevel() { return _curIndentLevel; }

private:
	void exportDocumentImpl(CADocument *doc);
	void exportSheetImpl(CASheet *sheet);
	void exportScoreBlock(CASheet *sheet);
	void exportStaffVoices(CAStaff *staff);
	void exportVoiceImpl(CAVoice *voice);
	void exportLyricsContextBlock(CALyricsContext *lc);
	void exportLyricsContextImpl(CALyricsContext* lc);
	void exportMarks( CAMusElement* );
	void exportNoteMarks( CANote* );
	void exportVolta( CAMusElement* );
	void exportPlayable( CAPlayable *elt );

	void writeDocumentHeader();
	CADiatonicPitch writeRelativeIntro();
	void doAnacrusisCheck(CATimeSignature *time);

	////////////////////
	// Helper methods //
	////////////////////
	const QString clefTypeToLilyPond( CAClef::CAClefType type, int c1, int offset );
	const QString diatonicKeyGenderToLilyPond( CADiatonicKey::CAGender gender);
	const QString playableLengthToLilyPond( CAPlayableLength length );
	const QString diatonicPitchToLilyPond( CADiatonicPitch p );
	const QString restTypeToLilyPond(CARest::CARestType type);
	const QString barlineTypeToLilyPond(CABarline::CABarlineType type);
	const QString syllableToLilyPond( CASyllable *s );

	inline const QString relativePitchToString(CANote* note, CADiatonicPitch prevPitch) {
		return relativePitchToString( note->diatonicPitch(), prevPitch);
	}
	const QString relativePitchToString( CADiatonicPitch p, CADiatonicPitch prevPitch);
	void voiceVariableName( QString &name, int staffNum, int voiceNum );
	void spellNumbers( QString &s );

	QString markupString( QString );
	QString escapeWeirdChars( QString );

	void indent();
	inline void indentMore() { ++_curIndentLevel; }
	inline void indentLess() { --_curIndentLevel; }

	///////////////////////////
	// Getter/Setter methods //
	///////////////////////////
	inline void setCurVoice(CAVoice *voice) { _curVoice = voice; }
	inline void setCurSheet(CASheet *sheet) { _curSheet = sheet; }
	inline void setCurContext(CAContext *context) { _curContext = context; }
	inline void setCurContextIndex(int c) { _curContextIndex = c; }
	inline void setCurDocument(CADocument *document) { _curDocument = document; }
	inline void setIndentLevel( int level) { _curIndentLevel = level; }

	/////////////
	// Members //
	/////////////
	QTextStream *_out;
	CAVoice *_curVoice;
	CASheet *_curSheet;
	CAContext *_curContext;
	CADocument *_curDocument;
	int _curContextIndex;
	int _curIndentLevel;

	// Voice exporting current status
	CADiatonicPitch _lastNotePitch;
	CAPlayableLength _lastPlayableLength;
	int _curStreamTime;

	void voltaFunction( void );
	bool _voltaFunctionWritten;
	bool _voltaBracketFinishAtRepeat;
	bool _voltaBracketFinishAtBar;
	static const QString _regExpVoltaRepeat;
	static const QString _regExpVoltaBar;
	bool _timeSignatureFound;
};

#endif /* LILYPONDEXPORT_H_*/
