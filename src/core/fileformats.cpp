/*! 
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QObject>
#include "core/fileformats.h"

/*!
	\class CAFileFormats
	\brief File formats supported by Canorus
	This class contains the filters shown in file dialogs (eg. when opening/saving a document) and its internal
	enumeration values used when storing settings for default or last used filter.
*/

const QString CAFileFormats::CANORUSML_FILTER = QObject::tr("Canorus document (*.xml)");
const QString CAFileFormats::LILYPOND_FILTER  = QObject::tr("LilyPond document (*.ly)");
const QString CAFileFormats::MUSICXML_FILTER  = QObject::tr("MusicXML document (*.xml)");
const QString CAFileFormats::NOTEEDIT_FILTER  = QObject::tr("NoteEdit document (*.not)");
const QString CAFileFormats::ABCMUSIC_FILTER  = QObject::tr("ABC music document (*.abc)");
const QString CAFileFormats::FINALE_FILTER    = QObject::tr("Finale document (*.mus)");
const QString CAFileFormats::SIBELIUS_FILTER  = QObject::tr("Sibelius document (*.sib)");
const QString CAFileFormats::CAPELLA_FILTER   = QObject::tr("Capella document (*.cap)");
const QString CAFileFormats::MIDI_FILTER      = QObject::tr("Midi file (*.mid, *.midi)");

/*!
	Converts the file format enumeration to filter as string.
*/
const QString CAFileFormats::getFilter( const CAFileFormats::CAFileFormatType t ) {
	switch (t) {
		case CanorusML:
			return CANORUSML_FILTER;
		case LilyPond:
			return LILYPOND_FILTER;
		case MusicXML:
			return MUSICXML_FILTER;
		default:
			return CANORUSML_FILTER;
	}
}

/*!
	Converts the file format filter string to enumeration appropriate for storing in config file.
*/
const CAFileFormats::CAFileFormatType CAFileFormats::getType( const QString t ) {
	if (t==CANORUSML_FILTER)
		return CanorusML;
	else
	if (t==LILYPOND_FILTER)
		return LilyPond;
	else
	if (t==MUSICXML_FILTER)
		return MusicXML;
	else
		return CanorusML;
}
