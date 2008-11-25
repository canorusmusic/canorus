/*!
	Copyright (c) 2006-2008, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef FILEFORMATS_H_
#define FILEFORMATS_H_

#include <QString>

class CAFileFormats {
public:
	enum CAFileFormatType {
		CanorusML  = 0,
		Can        = 1,
		LilyPond   = 2,
		MusicXML   = 3,
		ABCMusic   = 4,
		NoteEdit   = 5,
		MUP        = 6,
		Finale     = 7,
		Sibelius   = 8,
		Noteworthy = 9,
		Igor       = 10,
		Capella    = 11,
		Midi       = 12,
		PDF        = 13,
		SVG        = 14
	};

	static const QString LILYPOND_FILTER;
	static const QString CANORUSML_FILTER;
	static const QString CAN_FILTER;
	static const QString MUSICXML_FILTER;
	static const QString NOTEEDIT_FILTER;
	static const QString ABCMUSIC_FILTER;
	static const QString FINALE_FILTER;
	static const QString SIBELIUS_FILTER;
	static const QString CAPELLA_FILTER;
	static const QString MIDI_FILTER;
	static const QString PDF_FILTER;
	static const QString SVG_FILTER;

	static const QString getFilter( const CAFileFormatType );
	static const CAFileFormatType getType( const QString );
};

#endif /*FILEFORMATS_H_*/
