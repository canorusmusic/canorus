/*
 * Copyright (c) 2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 *
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#ifndef LILYPONDEXPORT_H_
#define LILYPONDEXPORT_H_

#include <QTextStream>
#include <QString>

#include "core/keysignature.h"
#include "core/timesignature.h"
#include "core/clef.h"
#include "core/barline.h"
#include "core/note.h"
#include "core/rest.h"
#include "core/document.h"
#include "core/lyricscontext.h"
#include "core/syllable.h"

class CALilyPondExport {
public:
	// Export the whole document constructor
	CALilyPondExport(CADocument *doc, QTextStream *out);
	// Export voice constructor
	CALilyPondExport(CAVoice *voice, QTextStream *out);
	// Export lyrics context constructor
	CALilyPondExport(CALyricsContext *lc, QTextStream *out);
	
	///////////////////////////
	// Polling export status //
	///////////////////////////
	// Setter methods are private!
	inline CAVoice *curVoice() { return _curVoice; }
	inline CASheet *curSheet() { return _curSheet; }
	inline CAContext *curContext() { return _curContext; }
	inline int curContextIndex() { return _curContextIndex; }
	inline int curIndentLevel() { return _curIndentLevel; }
	
private:
	void exportDocument(CADocument *doc);
	void exportSheet(CASheet *sheet);
	void exportScoreBlock(CASheet *sheet);
	void exportStaffVoices(CAStaff *staff);
	void exportVoice(CAVoice *voice);
	void exportLyricsContext(CALyricsContext *lc);
	void exportSyllables(CALyricsContext* lc);
	int writeRelativeIntro();
	
	////////////////////
	// Helper methods //
	////////////////////
	const QString clefTypeToLilyPond(CAClef::CAClefType type);	
	const QString keySignaturePitchToLilyPond(signed char accs, CAKeySignature::CAMajorMinorGender gender);
	const QString keySignatureGenderToLilyPond(CAKeySignature::CAMajorMinorGender gender);
	const QString playableLengthToLilyPond(CAPlayable::CAPlayableLength length, int dotted);
	const QString notePitchToLilyPond(int pitch, signed char accs);
	const QString restTypeToLilyPond(CARest::CARestType type);
	const QString barlineTypeToLilyPond(CABarline::CABarlineType type);
	const QString syllableToLilyPond( CASyllable *s );
	
	inline const QString relativePitchToString(CANote* note, int prevPitch) {
		return relativePitchToString(note->pitch(), note->accidentals(), prevPitch);
	}
	const QString relativePitchToString(int pitch, signed char accs, int prevPitch);
	void voiceVariableName( QString &name, int staffNum, int voiceNum );
	void spellNumbers( QString &s );
	
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
	inline void setIndentLevel( int level) { _curIndentLevel = level; }

	inline QTextStream& out() { return *_out; }
	
	/////////////
	// Members //
	/////////////
	QTextStream *_out;
	CAVoice *_curVoice;
	CASheet *_curSheet;
	CAContext *_curContext;
	int _curContextIndex;
	int _curIndentLevel;
};

#endif /* LILYPONDEXPORT_H_*/