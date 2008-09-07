/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QRegExp>
#include <QFileInfo>
#include <QTextStream>
#include <iostream>
#include <iomanip>
#include <stdio.h>

#include "export/midiexport.h"

#include "core/document.h"
#include "core/sheet.h"
#include "core/staff.h"
#include "core/voice.h"
#include "interface/playback.h"
#include "interface/mididevice.h"

class CACanorus;

/*!
	\class CAMidiExport
	\brief Midi file export filter
	This class is used to export the document or parts of the document to a midi file.
	The most common use is to simply call one of the constructors
	\code
	CAMidiExport( myDocument, &textStream );
	\endcode

	\a textStream is usually the file stream.

	\sa CAMidiImport
*/

/*!
	Constructor for midi file export. Called when choosing the mid/midi file extension in the export dialog.
	Exports all voices to the given text stream.
*/
CAMidiExport::CAMidiExport( QTextStream *out )
 : CAExport(out), CAMidiDevice() {
	_midiDeviceType = MidiExportDevice;
	setRealTime(false);
	trackTime = 0;
}

void CAMidiExport::send(QVector<unsigned char> message, int time)
{
	int offset = 0;
	if ( time > trackTime ) {
		offset = time-trackTime;
		trackTime = time;
	}
	if ( message.size() ) trackChunk.append( writeTime( offset ));
	char q;
	for (int i=0; i< message.size(); i++ ) {
		q = message[i];
		trackChunk.append(q);
	}
}

// FIXME: these magic numbers should go into the midi class.
#define META_TEXT        0x01
#define META_TIMESIG     0x58
#define META_KEYSIG      0x59
#define META_TEMPO       0x51
#define META_TRACK_END   0x2f

#define MIDI_CTL_EVENT   0xff
#define MIDI_CTL_REVERB  0x5b
#define MIDI_CTL_CHORUS  0x5d
#define MIDI_CTL_PAN     0x0a
#define MIDI_CTL_VOLUME  0x07
#define MIDI_CTL_SUSTAIN 0x40

// FIXME: is not yet fixed and synchronized to midi playback and import:
#define MULTIPLICATOR       (1*2*3*4*5*6*7) /* enable x-tuplets with x in {3,4,5,6,7,8,9,10} */
#define QUARTER_LENGTH      ( 32*MULTIPLICATOR)
#define TICKS_PER_QUARTER (3*128)

#define MY2MIDITIME(t) ((unsigned int) ((((double) t) * (double) (TICKS_PER_QUARTER)) / ((double) (QUARTER_LENGTH))))


QByteArray CAMidiExport::word16(int x) {
	QByteArray ba;
	ba.append(x >> 8);
	ba.append(x);
	return ba;
}



QByteArray CAMidiExport::variableLengthValue(int value) {

	QByteArray chunk;
	char b;
	bool byteswritten = false;
	b = (value >> 3*7) & 0x7f;
	if (b) {
		chunk.append(0x80 | b);
		byteswritten = true;
	}
	b = (value >> 2*7) & 0x7f;
	if (b || byteswritten) {
		chunk.append(0x80 | b);
		byteswritten = true;
	}
	b = (value >> 7) & 0x7f;
	if (b || byteswritten) {
		chunk.append(0x80 | b);
		byteswritten = true;
	}
	b = value & 0x7f;
	chunk.append(b);
	return chunk;
}



QByteArray CAMidiExport::writeTime(int time) {
	unsigned char b;
	bool byteswritten = false;
	QByteArray ba;

	b = (time >> 3*7) & 0x7f;
	if (b) {
		ba.append(0x80 | b);
		byteswritten = true;
	}
	b = (time >> 2*7) & 0x7f;
	if (b || byteswritten) {
		ba.append(0x80 | b);
		byteswritten = true;
	}
	b = (time >> 7) & 0x7f;
	if (b || byteswritten) {
		ba.append(0x80 | b);
		byteswritten = true;
	}
	b = time & 0x7f;
	ba.append(b);
	return ba;
}


QByteArray CAMidiExport::keySignature(void) {
	QByteArray tc;
	tc.append(writeTime(0));
	tc.append(MIDI_CTL_EVENT);
	tc.append(META_KEYSIG);
	tc.append(variableLengthValue( 2 ));
	tc.append((char)0);		// number of sharps, negative: number of flats positive
	tc.append((char)0);		// 0: major 1: minor
	return tc;
}


QByteArray CAMidiExport::timeSignature(void) {
	QByteArray tc;
	tc.append(writeTime(0));
	tc.append(MIDI_CTL_EVENT);
	tc.append(META_TIMESIG);
	tc.append(variableLengthValue( 4 ));
	tc.append(4);			// FIXME: this is just a fixed filled in numbers
	tc.append(2);
	tc.append(1);
	tc.append(8);
	return tc;
}


QByteArray CAMidiExport::trackEnd(void) {
	QByteArray tc;
	tc.append(writeTime(0));
	tc.append(MIDI_CTL_EVENT);
	tc.append(META_TRACK_END);
	tc.append((char)0);
	return tc;
}


QByteArray CAMidiExport::textEvent(int time, const char *s) {
	QByteArray tc;
	tc.append(writeTime(time));
	tc.append(MIDI_CTL_EVENT);
	tc.append(META_TEXT);
	tc.append(variableLengthValue(strlen(s)));
	tc.append(s);
	return tc;
}

/*!
	Exports the current document to Lilypond syntax as a complete .ly file.
*/
void CAMidiExport::exportDocumentImpl(CADocument *doc)
{
	if ( doc->sheetCount() < 1 ) {
		//TODO: no sheets, raise an error
		return;
	}


	// In the header chunk we need to know the count of tracks.
	// We export every non empty voice as separate track.
	// For now we export only the first sheet.
	CASheet *sheet = doc->sheetAt( 0 );
	setCurSheet( sheet );
	trackChunk.clear();

	// Let's playback this sheet and dump that into a file,
	// and for this we have our own midi driver.
	CAPlayback *_playback = new CAPlayback(sheet, this );
	_playback->run();

	int count = 0;
	for (int c = 0; c < doc->sheetAt(0)->contextCount(); ++c ) {
		switch (sheet->contextAt(c)->contextType()) {
			case CAContext::Staff:
				// exportStaffVoices( static_cast<CAStaff*>(sheet->contextAt( c )) );
				CAStaff *staff = static_cast<CAStaff*>(sheet->contextAt( c ));
				for ( int v = 0; v < staff->voiceCount(); ++v ) {
					setCurVoice( staff->voiceAt( v ) );
					count++;
					//std::cout << "Hallo  " << c << " " << v << "\n" << std::endl;
				}
		}
	}

	writeFile();
}

void CAMidiExport::writeFile() {
	// Header Chunk

	// A midi file here is 8-Bit Ascii, so we need no coding translation,
	// and this seems to switch it off, but I think there should be a null codec:   FIXME  !!
	(*stream()).setCodec("Latin-1");

	QByteArray headerChunk;
	headerChunk.append("MThd....");		// header and space for length
	headerChunk.append(word16( 1 ));	// Midi-Format version
	headerChunk.append(word16( 2 ));	// number of tracks, a control track and a music track for a trying out ...
//	headerChunk.append(word16( 384 ));	// time division, should be TICKS_PER_QUARTER
	headerChunk.append(word16( 512 ));  // time division 512 seems to work fine for us. Dunno why? -Matevz
	setChunkLength( &headerChunk );
	out() << headerChunk;


	QByteArray controlTrackChunk;
	controlTrackChunk.append( "MTrk...." );
	controlTrackChunk.append( textEvent(0, "Canorus Version 0.5beta generated. "));
	controlTrackChunk.append( textEvent(0, "Timebase and some midi controls not yet implemented."));
	controlTrackChunk.append( trackEnd());
	setChunkLength( &controlTrackChunk );
	//printQByteArray( controlTrackChunk );
	out() << controlTrackChunk;

	// trackChunk is already filled with midi data,
	// let's add chunk header, in reverse, ...
	trackChunk.prepend(keySignature());
	trackChunk.prepend(timeSignature());
	trackChunk.prepend("MTrk....");
	// ... and add the tail:
	trackChunk.append(trackEnd());
	setChunkLength( &trackChunk );
	//printQByteArray( trackChunk );
	out() << trackChunk;
}

void CAMidiExport::setChunkLength( QByteArray *x ) {
	quint32 l = (*x).size() - 8;	// subtract header length
	for (int i=0; i<4; i++ ) {
		(*x )[7-i] = l >> (8*i);
	}
}


void CAMidiExport::printQByteArray( QByteArray x )
{
	for (int i=0; i<x.size(); i++ ) {
		printf( " %02x", 0x0ff & x.at(i));
	}
	printf( "\n");
}



