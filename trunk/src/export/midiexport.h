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
	void send(QVector<unsigned char> message, int time);
	
/*
	///////////////////////////
	// Polling export status //
	///////////////////////////
	// Setter methods are private!
*/
	
private:
	QByteArray writeTime(int time);
	void exportDocumentImpl(CADocument *doc);
	int midiTrackCount;
	QByteArray trackChunk;					// for the time beeing we build one big track
	int trackTime;							// which this is the time line for
	QVector<QByteArray> trackChunks;		// for the future
	QVector<int> trackTimes;
	void printQByteArray( QByteArray x );	// for debugging only
	QByteArray variableLengthValue(int value);
	QByteArray word16(int x);
	QByteArray textEvent(int time, const char *s);
	QByteArray trackEnd(void);
	QByteArray timeSignature(void);
	QByteArray keySignature(void);
	void setChunkLength( QByteArray *x );

/*
	
	////////////////////
	// Helper methods //
	////////////////////
	
	///////////////////////////
	// Getter/Setter methods //
	///////////////////////////
*/
	inline void setCurVoice(CAVoice *voice) { _curVoice = voice; }
	inline void setCurSheet(CASheet *sheet) { _curSheet = sheet; }
/*
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
