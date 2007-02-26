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

class CALilyPondExport {
public:
	// Export voice constructor
	CALilyPondExport(CAVoice *voice, QTextStream *out);
	
	///////////////////////////
	// Polling export status //
	///////////////////////////
	inline CAVoice *curVoice() { return _curVoice; } // Setter method is private!

private:
	void exportVoice(CAVoice *voice);
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
	
	inline const QString relativePitchToString(CANote* note, int prevPitch) {
		return relativePitchToString(note->pitch(), note->accidentals(), prevPitch);
	}
	const QString relativePitchToString(int pitch, signed char accs, int prevPitch);
	
	///////////////////////////
	// Getter/Setter methods //
	///////////////////////////
	inline void setCurVoice(CAVoice *voice) { _curVoice = voice; }
	inline QTextStream& out() { return *_out; }
	
	/////////////
	// Members //
	/////////////
	QTextStream *_out;
	CAVoice *_curVoice;
};

#endif /* LILYPONDEXPORT_H_*/
