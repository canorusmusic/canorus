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

class CALilyPondExport {
public:
	// Export voice constructor
	CALilyPondExport(CAVoice *voice, QTextStream *out);

	// Export document constructor
	CALilyPondExport(CADocument *doc, QTextStream *out);
	
	///////////////////////////
	// Polling export status //
	///////////////////////////

	// Setter methods are private!
	inline CAVoice *curVoice() { return _curVoice; }
	inline CASheet *curSheet() { return _curSheet; }
	inline CAStaff *curStaff() { return _curStaff; }
	inline int curIndentLevel() { return _curIndentLevel; }
	
private:
	void exportVoice(CAVoice *voice);
	void exportSheet(CASheet *sheet);
	void exportDocument(CADocument *doc);
	void exportStaffVoices(CAStaff *staff);
	void exportScoreBlock(CASheet *sheet);
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
	
	inline const QString relativePitchToString(CANote* note, int prevPitch) {
		return relativePitchToString(note->pitch(), note->accidentals(), prevPitch);
	}
	const QString relativePitchToString(int pitch, signed char accs, int prevPitch);
	void spellNumbers( QString &s );
	
	void indent();
	inline void indentMore() { ++_curIndentLevel; }
	inline void indentLess() { --_curIndentLevel; }
	
	///////////////////////////
	// Getter/Setter methods //
	///////////////////////////
	inline void setCurVoice(CAVoice *voice) { _curVoice = voice; }
	inline void setCurSheet(CASheet *sheet) { _curSheet = sheet; }
	inline void setCurStaff(CAStaff *staff) { _curStaff = staff; }
	inline void setIndentLevel( int level) { _curIndentLevel = level; }

	inline QTextStream& out() { return *_out; }
	
	/////////////
	// Members //
	/////////////
	QTextStream *_out;
	CAVoice *_curVoice;
	CASheet *_curSheet;
	CAStaff *_curStaff;
	int _curIndentLevel;
};

#endif /* LILYPONDEXPORT_H_*/
