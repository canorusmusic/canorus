/*! 
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
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
		LilyPond   = 1,
		MusicXML   = 2,
		ABCMusic   = 3,
		NoteEdit   = 4,
		MUP        = 5,
		Finale     = 6,
		Sibelius   = 7,
		Noteworthy = 8,
		Igor       = 9,
		Capella    = 10,
		Midi       = 11
	};
	
	static const QString LILYPOND_FILTER;
	static const QString CANORUSML_FILTER;
	static const QString MUSICXML_FILTER;
	static const QString NOTEEDIT_FILTER;
	static const QString ABCMUSIC_FILTER;
	static const QString FINALE_FILTER;
	static const QString SIBELIUS_FILTER;
	static const QString CAPELLA_FILTER;
	static const QString MIDI_FILTER;
	
	static const QString getFilter( const CAFileFormatType );
	static const CAFileFormatType getType( const QString );
};

#endif /*FILEFORMATS_H_*/
