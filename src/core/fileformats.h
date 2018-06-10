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
		CanorusML  = 1,
		Can        = 2,
		LilyPond   = 3,
		MusicXML   = 4,
        MXL        = 16,
		ABCMusic   = 5,
		NoteEdit   = 6,
		MUP        = 7,
		Finale     = 8,
		Sibelius   = 9,
		Noteworthy = 10,
		Igor       = 11,
		Capella    = 12,
		Midi       = 13,
		PDF        = 14,
		SVG        = 15
	};

	static const QString LILYPOND_FILTER;
	static const QString CANORUSML_FILTER;
	static const QString CAN_FILTER;
	static const QString MUSICXML_FILTER;
    static const QString MXL_FILTER;
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
