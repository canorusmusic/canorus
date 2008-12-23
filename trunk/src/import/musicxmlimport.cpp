/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QDebug>
#include <QXmlStreamAttributes>
#include "import/musicxmlimport.h"

#include "import/canorusmlimport.h"

#include "core/document.h"
#include "core/sheet.h"
#include "core/context.h"
#include "core/staff.h"
#include "core/voice.h"
#include "core/playable.h"
#include "core/note.h"
#include "core/rest.h"
#include "core/clef.h"
#include "core/muselement.h"
#include "core/keysignature.h"
#include "core/timesignature.h"
#include "core/barline.h"

#include "core/mark.h"
#include "core/text.h"
#include "core/tempo.h"
#include "core/bookmark.h"
#include "core/articulation.h"
#include "core/crescendo.h"
#include "core/instrumentchange.h"
#include "core/dynamic.h"
#include "core/ritardando.h"
#include "core/fermata.h"
#include "core/repeatmark.h"
#include "core/fingering.h"

#include "core/lyricscontext.h"
#include "core/syllable.h"

#include "core/functionmarkcontext.h"
#include "core/functionmark.h"

CAMusicXmlImport::CAMusicXmlImport( QTextStream *stream )
 : CAImport(stream), QXmlStreamReader() {
	initMusicXmlImport();
}

CAMusicXmlImport::CAMusicXmlImport( const QString stream )
 : CAImport(stream), QXmlStreamReader() {
	initMusicXmlImport();
}

CAMusicXmlImport::~CAMusicXmlImport() {
}

void CAMusicXmlImport::initMusicXmlImport() {
	_document = 0;
}

/*!
	Opens a MusicXML source \a in and creates a document out of it.
	CAMusicXmlImport uses SAX model for reading.
*/
CADocument* CAMusicXmlImport::importDocumentImpl() {
	QXmlStreamReader::setDevice( stream()->device() );

	while (!atEnd()) {
		readNext();

		if (error()) {
			setStatus( -2 );
			break;
		}

		switch (tokenType()) {
		case StartDocument:
		case DTD: {
			readHeader();
			break;
		}
		case StartElement: {
			if ( name()=="score-partwise" ) {
				_musicXmlVersion=attributes().value("version").toString();
				readScorePartwise();
			} else
			if ( name()=="score-timewise" ) {
				_musicXmlVersion=attributes().value("version").toString();
				readScoreTimewise();
			}
			break;
		}
		}
	}

	return _document;
}

const QString CAMusicXmlImport::readableStatus() {
	if (status()==-2) {
		return errorString();
	} else {
		return CAImport::readableStatus();
	}
}

void CAMusicXmlImport::readHeader() {
	if (tokenType()==DTD) {
		if ( dtdName()!="score-partwise" && dtdName()!="score-timewise" ) {
			raiseError( tr("File is not a correct MusicXML file.") );
		}
	}
}

void CAMusicXmlImport::readScorePartwise() {
	if (name()!="score-partwise") return;

	_document = new CADocument();

	while (!atEnd() && !(tokenType()==EndElement && name()=="score-partwise")) {
		readNext();

		if (name()=="work") {
			readWork();
		} else if (name()=="identification") {
			readIdentification();
		} else if (name()=="defaults") {
			readDefaults();
		} else if (name()=="part-list") {
			readPartList();
		} else if (name()=="part") {
			readPart();
		}
	}

	for (int i=0; i<_document->sheetAt(0)->staffCount(); i++) {
		for (int j=0; j<_document->sheetAt(0)->staffAt(i)->voiceCount(); j++) {
			_document->sheetAt(0)->staffAt(i)->voiceAt(j)->setMidiProgram( _midiProgram[_document->sheetAt(0)->staffAt(i)] );
			_document->sheetAt(0)->staffAt(i)->voiceAt(j)->setMidiChannel( _midiChannel[_document->sheetAt(0)->staffAt(i)] );
		}
	}
}

void CAMusicXmlImport::readScoreTimewise() {
	if (name()!="score-timewise") return;

	_document = new CADocument();

	// TODO: No support for score-timewise yet
}

void CAMusicXmlImport::readWork() {
	if (name()!="work") return;

	while (!atEnd() && !(tokenType()==EndElement && name()=="work")) {
		readNext();

		if (tokenType()==StartElement) {
			if (name()=="work-title") {
				_document->setTitle( readElementText() );
			}
		}
	}
}

void CAMusicXmlImport::readDefaults() {
	if (name()!="defaults") return;

	// TODO: Currently this just ignores the defaults
	while (!atEnd() && !(tokenType()==EndElement && name()=="defaults")) {
		readNext();
	}
}

void CAMusicXmlImport::readIdentification() {
	if (name()!="identification") return;

	while (!atEnd() && !(tokenType()==EndElement && name()=="identification")) {
		readNext();

		if (tokenType()==StartElement) {
			if (name()=="creator" && attributes().value("type")=="composer") {
				_document->setComposer( readElementText() );
			} else
			if (name()=="creator" && attributes().value("type")=="lyricist") {
				_document->setPoet( readElementText() );
			}
			if (name()=="rights") {
				_document->setCopyright( readElementText() );
			}
		}
	}
}

void CAMusicXmlImport::readPartList() {
	if (name()!="part-list") return;

	CASheet *sheet = _document->addSheet();

	while (!atEnd() && !(tokenType()==EndElement && name()=="part-list")) {
		readNext();

		if (tokenType()==StartElement) {
			CAStaff *staff=0;
			QString id;
			if (name()=="score-part") {
				id = attributes().value("id").toString();
				staff = _document->sheetAt(0)->addStaff();

				while (!atEnd() && !(tokenType()==EndElement && name()=="score-part")) {
					readNext();

					if (tokenType()==StartElement && name()=="part-name") {
						staff->setName( readElementText() );
					} else if (tokenType()==StartElement && name()=="midi-channel") {
						_midiChannel[staff] = readElementText().toInt();
					} else if (tokenType()==StartElement && name()=="midi-program") {
						_midiProgram[staff] = readElementText().toInt();
					}
				}
			}

			if (staff) {
				_staffMap[ id ] = staff;
			}
		}
	}
}

void CAMusicXmlImport::readPart() {
	if (name()!="part") return;

	CAStaff *staff = _staffMap[attributes().value("id").toString()];
	while (!atEnd() && !(tokenType()==EndElement && name()=="part")) {
		readNext();

		if (tokenType()==StartElement) {
			if (name()=="measure") {
				readMeasure(staff);
			}
		}
	}

	staff->synchronizeVoices();
}

void CAMusicXmlImport::readMeasure( CAStaff *staff ) {
	if (name()!="measure") return;

	while (!atEnd() && !(tokenType()==EndElement && name()=="measure")) {
		readNext();

		if (tokenType()==StartElement) {
			if (name()=="attributes") {
				readAttributes( staff );
			} else  if (name()=="note") {
				readNote( staff, _divisions[staff] );
			}
		}
	}

	staff->voiceAt(0)->append( new CABarline( CABarline::Single, staff, 0 ) );
}

void CAMusicXmlImport::readAttributes( CAStaff *staff ) {
	if (name()!="attributes") return;

	while (!atEnd() && !(tokenType()==EndElement && name()=="attributes")) {
		readNext();

		if (tokenType()==StartElement) {
			if (name()=="divisions") {

				_divisions[staff] = readElementText().toInt();

			} else if (name()=="key") {

				while (name()!="fifths") readNext();
				int accs = readElementText().toInt();
				while (name()!="mode") readNext();
				CADiatonicKey::CAGender gender = CADiatonicKey::genderFromString( readElementText() );

				staff->voiceAt(0)->append( new CAKeySignature( CADiatonicKey( accs, gender), staff, 0 ) );

			} else if (name()=="time") {

				while (name()!="beats") readNext();
				int beats = readElementText().toInt();
				while (name()!="beat-type") readNext();
				int beat = readElementText().toInt();

				staff->voiceAt(0)->append( new CATimeSignature( beats, beat, staff, 0 ) );

			} else if (name()=="clef") {

				while (name()!="sign") readNext();
				QString sign = readElementText();

				CAClef::CAPredefinedClefType t;
				if (sign=="G") t=CAClef::Treble;
				else if (sign=="F") t=CAClef::Bass;

				staff->voiceAt(0)->append( new CAClef( t, staff, 0 ) );

			}
		}
	}
}

void CAMusicXmlImport::readNote( CAStaff *staff, int divisions ) {
	if (name()!="note") return;

	bool isRest = false;
	bool isPartOfChord = false;
	int voice = -1;
	CAPlayableLength length;
	CADiatonicPitch pitch;
	CANote::CAStemDirection stem = CANote::StemPreferred;

	while (!atEnd() && !(tokenType()==EndElement && name()=="note")) {
		readNext();

		if (tokenType()==StartElement) {
			if (name()=="rest") {
				isRest = true;
			} else if (name()=="chord") {
				isPartOfChord = true;
			} else if (name()=="type") {
				length = CAPlayableLength::musicLengthFromString( readElementText() );
			} else if (name()=="stem") {
				QString s = readElementText();
				if (s=="up") stem = CANote::StemUp;
				else if (s=="down") stem = CANote::StemDown;
			} else if (name()=="pitch") {
				while (name()!="step") readNext();
				QString step = readElementText();
				while (name()!="octave") readNext();
				int octave = readElementText().toInt();

				pitch = CADiatonicPitch::diatonicPitchFromString( step );
				pitch.setNoteName( pitch.noteName()+(octave*7) );
			} else if (name()=="voice") {
				voice = readElementText().toInt();
			}
		}
	}

	if (voice!=-1) {
		while (voice > staff->voiceCount()) {
			staff->addVoice();
		}

		CAVoice *v = staff->voiceAt(voice-1);
		CAPlayable *p;
		if (!isRest) {
			p = new CANote( pitch, length, v, 0 );
		} else {
			p = new CARest( CARest::Normal, length, v, 0 );
		}

		v->append( p, isPartOfChord );
	}
}
