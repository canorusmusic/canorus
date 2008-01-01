/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef MIDIEXPORT_H_
#define MIDIEXPORT_H_

#include <QTextStream>
#include <QString>
#include <QList>
#include <QVector>
#include <QByteArray>

#include "core/keysignature.h"
#include "core/timesignature.h"
#include "core/clef.h"
#include "core/barline.h"
#include "core/note.h"
#include "core/rest.h"
#include "core/document.h"
#include "core/lyricscontext.h"
#include "core/syllable.h"

#include "export/export.h"
#include "interface/mididevice.h"
#include "interface/playback.h"


class CAMidiExport : public CAExport, public CAMidiDevice {
public:
	CAMidiExport( QTextStream *out=0 );

/*
    //////////////////////////////
    // Interface to file export //
    //////////////////////////////
*/
	QMap<int, QString> getOutputPorts() { };
	QMap<int, QString> getInputPorts() { };
	
	bool openOutputPort(int port) { return true; }	// return true on success, false otherwise
	bool openInputPort(int port) { return true; }	// return true on success, false otherwise	
	void closeOutputPort() { }
	void closeInputPort() { }
	void send(QVector<unsigned char> message, int offset);
	
/*
	///////////////////////////
	// Polling export status //
	///////////////////////////
	// Setter methods are private!
	inline CAVoice *curVoice() { return _curVoice; }
	inline CASheet *curSheet() { return _curSheet; }
	inline CAContext *curContext() { return _curContext; }
	inline int curContextIndex() { return _curContextIndex; }
	inline int curIndentLevel() { return _curIndentLevel; }
*/
	
private:
	void writeWord(unsigned int w);
	void writeDWord(unsigned int w);
	void writeString(char *s);
	void writeTime(int time);
	void writeText(int time, char *s);
	void exportDocumentImpl(CADocument *doc);
	int midiTrackCount;
	QVector<QByteArray> trackChunks;
	//void exportVoiceImpl(CAVoice *voice, QByteArray *trackChunk);
/*
	void exportSheetImpl(CASheet *sheet);
	void exportScoreBlock(CASheet *sheet);
	void exportStaffVoices(CAStaff *staff);
	void exportLyricsContextImpl(CALyricsContext *lc);
	void exportSyllables(CALyricsContext* lc);
	
	void writeDocumentHeader();
	int writeRelativeIntro();
	
	////////////////////
	// Helper methods //
	////////////////////
	const QString clefTypeToLilyPond( CAClef::CAClefType type, int c1, int offset );	
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
	
	QString markupString( QString );
	QString escapeWeirdChars( QString );
	
	void indent();
	inline void indentMore() { ++_curIndentLevel; }
	inline void indentLess() { --_curIndentLevel; }
	
	///////////////////////////
	// Getter/Setter methods //
	///////////////////////////
*/
	inline void setCurVoice(CAVoice *voice) { _curVoice = voice; }
	inline void setCurSheet(CASheet *sheet) { _curSheet = sheet; }
/*
	inline void setCurContext(CAContext *context) { _curContext = context; }
	inline void setCurContextIndex(int c) { _curContextIndex = c; }
	inline void setIndentLevel( int level) { _curIndentLevel = level; }
	
	/////////////
	// Members //
	/////////////
	QTextStream *_out;
*/
	CAVoice *_curVoice;
	CASheet *_curSheet;
/*
	CAContext *_curContext;
	int _curContextIndex;
	int _curIndentLevel;
*/
};

#endif /* MIDIEXPORT_H_*/
