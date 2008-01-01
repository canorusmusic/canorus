/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QRegExp>
#include <QFileInfo>
#include <QTextStream>
#include <iostream>

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
	\brief LilyPond export filter
	This class is used to export the document or parts of the document to LilyPond syntax.
	The most common use is to simply call one of the constructors
	\code
	CAMidiExport( myDocument, &textStream );
	\endcode
	
	\a textStream is usually the file stream or the content of the score source view widget.
	
	\sa CALilyPondImport
*/

/*!
	Constructor for voice export. Called when viewing a single voice source in Lily syntax.
	Exports a voice to LilyPond syntax using the given text stream.
*/
CAMidiExport::CAMidiExport( QTextStream *out )
 : CAExport(out), CAMidiDevice() {
	_midiDeviceType = MidiExportDevice;
	setRealTime(false);
}

void CAMidiExport::send(QVector<unsigned char> message, int offset)
{
	std::cout << "Hallo    " << offset << "  ";
	for (int i=0; i< message.size(); i++ ) {
		out() << message[i];
		std::cout << message[i] << " ";
	}
	std::cout << std::endl;
}

/*!
	Exports the given voice music elements to LilyPond syntax.
	
	\sa CALilypondImport
*/
/*
void CAMidiExport::exportVoiceImpl(CAVoice *v, QByteArray *trackChunk) {
	
	// this function will be taken from CAPlayback,
	// that's more appropriate!
}
*/


#define META_TEXT    0x01
#define META_TIMESIG 0x58
#define META_KEYSIG  0x59
#define META_TEMPO   0x51

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


static unsigned char trackend[] = {0x00, 0xff , 0x2f, 0x00};


void CAMidiExport::writeWord(unsigned int w) {
	for (int i = 1; i >= 0; i-- ) {
		char c = (unsigned char)(w >> (i*8));
		out() << c;
	}
}


void CAMidiExport::writeDWord(unsigned int dw) {
	for (int i = 3; i >= 0; i-- ) {
		char c = (unsigned char)(dw >> (i*8));
		out() << c;
	}
}


void CAMidiExport::writeString(char *s) {
	while (*s) {
		out() << *s++;
	}
}

void CAMidiExport::writeTime(int time) {
	unsigned char b;
	bool byteswritten = false;

	b = (time >> 3*7) & 0x7f;
	if (b) {
		out() << (0x80 | b);
		byteswritten = true;
	}
	b = (time >> 2*7) & 0x7f;
	if (b || byteswritten) {
		out() << (0x80 | b);
		byteswritten = true;
	}
	b = (time >> 7) & 0x7f;
	if (b || byteswritten) {
		out() << (0x80 | b);
		byteswritten = true;
	}
	b = time & 0x7f;
	out() << b;
}


void CAMidiExport::writeText(int time, char *s) {
	char *cptr = s;
	writeTime(time);
	// writeByte(0xff); writeByte(META_TEXT); writeByte(strlen(s));
	char c = 0xff;
	out() << c;
	c = META_TEXT;
	out() << c;
	c = strlen(s);
	out() << c;
	writeString( s );
	//while (*cptr) {
	//	putc(0xff & (*cptr++), midiout_);
	//}
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

	// Let's playback this sheet and dump that into a file,
	// and for this we have our own midi driver:
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
					std::cout << "Hallo  " << c << " " << v << "\n" << std::endl;
				}
		}
	}

	// Header Chunk
	writeString("MThd");
	writeDWord(6);					// length
	writeWord(1);					// Midi-Format version
	writeWord(count + 1);			// number of tracks
	writeWord(TICKS_PER_QUARTER);	// time division

	// Track Chunks still needed from above
	writeString("MTrk");
	writeDWord(0);
}


/*!
	Exports the current sheet to Lilypond syntax.
*/
/*
void CAMidiExport::exportSheetImpl(CASheet *sheet)
{
	setCurSheet( sheet );

	// Export voices as Lilypond variables: \StaffOneVoiceOne = \relative c { ... }
	for ( int c = 0; c < sheet->contextCount(); ++c ) {
		setCurContextIndex( c );
		switch (sheet->contextAt(c)->contextType()) {
			case CAContext::Staff:
				// exportStaffVoices( static_cast<CAStaff*>(sheet->contextAt( c )) );
				break;
			case CAContext::LyricsContext:
				exportLyricsContextImpl( static_cast<CALyricsContext*>(sheet->contextAt( c )) );
				break;			
		}
	}

	exportScoreBlock( sheet );
}
*/

