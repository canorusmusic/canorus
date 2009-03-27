/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QDebug>
#include <QXmlStreamAttributes>
#include <iostream> // debug
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

		if (tokenType()==StartElement) {
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
	}

	for (int i=0; i<_partMapStaff.keys().size(); i++) {
		// go through all partIds
		for (int j=0; j<_partMapStaff[_partMapStaff.keys()[i]].size(); j++) {
			// go through all staffs with this partId
			CAStaff *s = _partMapStaff[_partMapStaff.keys()[i]][j];
			for (int k=0; k<s->voiceCount(); k++) {
				// go through all voices in this staff
				s->voiceAt(k)->setMidiProgram( _midiProgram[_partMapStaff.keys()[i]] );
				s->voiceAt(k)->setMidiChannel( _midiChannel[_partMapStaff.keys()[i]] );
			}
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
			QString partId;
			if (name()=="score-part") {
				partId = attributes().value("id").toString();

				while (!atEnd() && !(tokenType()==EndElement && name()=="score-part")) {
					readNext();

					if (tokenType()==StartElement && name()=="part-name") {
						_partName[partId] = readElementText();
					} else if (tokenType()==StartElement && name()=="midi-channel") {
						_midiChannel[partId] = readElementText().toInt();
					} else if (tokenType()==StartElement && name()=="midi-program") {
						_midiProgram[partId] = readElementText().toInt();
					}
				}
			}
		}
	}
}

void CAMusicXmlImport::readPart() {
	if (name()!="part") return;

	QString partId = attributes().value("id").toString();
	_partMapStaff[ partId ] = QList<CAStaff*>();
	_partMapClef[ partId ] = QHash<int, CAClef*>();
	_partMapKeySig[ partId ] = QHash<int, CAKeySignature*>();
	_partMapTimeSig[ partId ] = QHash<int, CATimeSignature*>();

	while (!atEnd() && !(tokenType()==EndElement && name()=="part")) {
		readNext();

		if (tokenType()==StartElement) {
			if (name()=="measure") {
				readMeasure(partId);
			}
		}
	}
}

void CAMusicXmlImport::readMeasure( QString partId ) {
	if (name()!="measure") return;

	while (!atEnd() && !(tokenType()==EndElement && name()=="measure")) {
		readNext();

		if (tokenType()==StartElement) {
			if (name()=="attributes") {
				readAttributes( partId );
			} else if (name()=="note") {
				readNote( partId, _divisions[partId] );
			} else if (name()=="forward") {
				readForward( partId, _divisions[partId] );
			}
		}
	}

	// Finish the measure (add barlines to all staffs)
	for (int staffIdx=0; staffIdx<_partMapStaff[partId].size(); staffIdx++) {
		CAStaff *staff = _partMapStaff[partId][staffIdx];
		int lastVoice=-1;
		for (int i=0; i<staff->voiceCount(); i++) {
			if ( lastVoice==-1 || staff->voiceAt(lastVoice)->lastTimeEnd() < staff->voiceAt(i)->lastTimeEnd() ) {
				lastVoice = i;
			}
		}

		if (lastVoice!=-1) {
			staff->voiceAt(lastVoice)->append( new CABarline( CABarline::Single, staff, 0 ) );
			staff->synchronizeVoices();
		}
	}
}

void CAMusicXmlImport::readAttributes( QString partId ) {
	if (name()!="attributes") return;

	int staves=1;

	while (!atEnd() && !(tokenType()==EndElement && name()=="attributes")) {
		readNext();

		if (tokenType()==StartElement) {
			if (name()=="divisions") {

				_divisions[partId] = readElementText().toInt();

			} else if (name()=="staves") {
				staves = readElementText().toInt();
			} else if (name()=="key") {

				int accs = 0;
				int number = (attributes().value("number").toString().isEmpty()?1:attributes().value("number").toString().toInt());
				CADiatonicKey::CAGender gender = CADiatonicKey::Major;

				while (!atEnd() && !(tokenType()==EndElement && name()=="key")) {
					readNext();

					if (tokenType()==StartElement) {
						if ( name()=="fifths" ) {
							accs = readElementText().toInt();
						}
						if ( name()=="mode" ) {
							gender = CADiatonicKey::genderFromString( readElementText() );
						}
					}
				}

				if (_partMapStaff[partId].size()>=number) {
					_partMapKeySig[partId][number] = new CAKeySignature( CADiatonicKey( accs, gender ), _partMapStaff[partId][number-1], 0 );
				} else {
					_partMapKeySig[partId][number] = new CAKeySignature( CADiatonicKey( accs, gender ), 0, 0 );
				}

			} else if (name()=="time") {

				int beats = 1;
				int beat = 1;
				int number = (attributes().value("number").toString().isEmpty()?1:attributes().value("number").toString().toInt());

				while (!atEnd() && !(tokenType()==EndElement && name()=="time")) {
					readNext();

					if (tokenType()==StartElement) {
						if ( name()=="beats" ) {
							beats= readElementText().toInt();
						}
						if ( name()=="beat-type" ) {
							beat = readElementText().toInt();
						}
					}
				}

				if (_partMapStaff[partId].size()>=number) {
					_partMapTimeSig[partId][number] = new CATimeSignature( beats, beat, _partMapStaff[partId][number-1], 0 );
				} else {
					_partMapTimeSig[partId][number] = new CATimeSignature( beats, beat, 0, 0 );
				}

			} else if (name()=="clef") {

				QString sign;
				int number = (attributes().value("number").toString().isEmpty()?1:attributes().value("number").toString().toInt());

				while (!atEnd() && !(tokenType()==EndElement && name()=="clef")) {
					readNext();

					if (tokenType()==StartElement) {
						if ( name()=="sign" ) {
							sign = readElementText();
						}
					}
				}

				CAClef::CAPredefinedClefType t;
				if (sign=="G") t=CAClef::Treble;
				else if (sign=="F") t=CAClef::Bass;

				if (_partMapStaff[partId].size()>=number) {
					_partMapClef[partId][number] = new CAClef( t, _partMapStaff[partId][number-1], 0 );
				} else {
					_partMapClef[partId][number] = new CAClef( t, 0, 0 );
				}

			}
		}
	}

	for (int i=1; i<=staves && staves > _partMapStaff[partId].size(); i++) {
		CAStaff *s = new CAStaff( tr("Staff%1").arg(_document->sheetAt(0)->staffCount()), _document->sheetAt(0) );
		_document->sheetAt(0)->addContext(s);
		_partMapStaff[partId].append( s );

		if (_partMapKeySig[partId].contains(i)) {
			_partMapKeySig[partId][i]->setContext( s );
		}
		if (_partMapTimeSig[partId].contains(i)) {
			_partMapTimeSig[partId][i]->setContext( s );
		}
		if (_partMapClef[partId].contains(i)) {
			_partMapClef[partId][i]->setContext( s );
		}
	}
}

void CAMusicXmlImport::readNote( QString partId, int divisions ) {
	if (name()!="note") return;

	bool isRest = false;
	bool isPartOfChord = false;
	int voice = 1;
	int staff = 1;
	CAPlayableLength length;
	CADiatonicPitch pitch;
	CANote::CAStemDirection stem = CANote::StemPreferred;
	int lyricsNumber=-1;
	QString lyricsText;

	while (!atEnd() && !(tokenType()==EndElement && name()=="note")) {
		readNext();

		if (tokenType()==StartElement) {
			if (name()=="rest") {
				isRest = true;
			} else if (name()=="chord") {
				isPartOfChord = true;
			} else if (name()=="duration") {
				int duration = readElementText().toInt();
				length = CAPlayableLength::timeLengthToPlayableLengthList( (duration/(float)divisions) * 256 ).first();
			} else if (name()=="stem") {
				QString s = readElementText();
				if (s=="up") stem = CANote::StemUp;
				else if (s=="down") stem = CANote::StemDown;
			} else if (name()=="pitch") {
				int alter = 0;
				QString step;
				int octave = -1;
				while (!atEnd() && !(tokenType()==EndElement && name()=="pitch")) {
					readNext();

					if (tokenType()==StartElement) {
						if (name()=="step") {
							step = readElementText();
						} else if (name()=="octave") {
							octave = readElementText().toInt();
						} else if (name()=="alter") {
							alter = readElementText().toInt();
						}
					}
				}

				pitch = CADiatonicPitch::diatonicPitchFromString( step );
				pitch.setNoteName( pitch.noteName()+(octave*7) );
				pitch.setAccs( alter );
			} else if (name()=="voice") {
				voice = readElementText().toInt();
			} else if (name()=="staff") {
				staff = readElementText().toInt();
			} else if (name()=="lyric") {
				lyricsNumber=1;

				if ( !attributes().value("number").isEmpty() ) {
					lyricsNumber = attributes().value("number").toString().toInt();
				}

				while (name()!="text") readNext();
				lyricsText = readElementText();
			}
		}
	}

	CAVoice *v = addVoiceIfNeeded( partId, staff, voice );

	CAPlayable *p;
	if (!isRest) {
		p = new CANote( pitch, length, v, 0 );
	} else {
		p = new CARest( CARest::Normal, length, v, 0 );
	}

	v->append( p, isPartOfChord );

	if (lyricsNumber!=-1) {
		while (lyricsNumber > v->lyricsContextList().size()) {
			v->addLyricsContext( new CALyricsContext( v->name()+tr("Lyrics"), v->lyricsContextList().size()+1, v ) );
			_document->sheetAt(0)->insertContextAfter( v->staff(), v->lyricsContextList()[lyricsNumber-1] );
		}

		v->lyricsContextList()[lyricsNumber-1]->addSyllable( new CASyllable(lyricsText, false, false, v->lyricsContextList()[lyricsNumber-1], p->timeStart(), p->timeLength() ) );
	}
}

CAVoice *CAMusicXmlImport::addVoiceIfNeeded( QString partId, int staff, int voice ) {
	CAVoice *v = 0;
	CAStaff *s = 0;

	if (!_partMapVoice[partId].contains(voice)) {
		s = _partMapStaff[partId][staff-1];
		v = new CAVoice( tr("Voice%1").arg(s->voiceCount()), s );
		if (!s->voiceCount()) {
			if (_partMapClef[partId].contains(staff)) {
				v->append(_partMapClef[partId][staff]);
			} else if (_partMapClef[partId].contains(1)) { // add the default clef
				v->append(_partMapClef[partId][1]->clone(s));
			}

			if (_partMapKeySig[partId].contains(staff)) {
				v->append(_partMapKeySig[partId][staff]);
			} else if (_partMapKeySig[partId].contains(1)) { // add the default keysig
				v->append(_partMapKeySig[partId][1]->clone(s));
			}

			if (_partMapTimeSig[partId].contains(staff)) {
				v->append(_partMapTimeSig[partId][staff]);
			} else if (_partMapTimeSig[partId].contains(1)) { // add the default timesig
				v->append(_partMapTimeSig[partId][1]->clone(s));
			}
		}

		s->addVoice(v);
		s->synchronizeVoices();
		_partMapVoice[partId][voice] = v;
	} else {
		v = _partMapVoice[partId][voice];
		s = v->staff();
	}

	return v;
}

void CAMusicXmlImport::readForward( QString partId, int divisions ) {
	if (name()!="forward") return;

	int voice=-1;
	int length=-1;
	int staff=1;

	while (!atEnd() && !(tokenType()==EndElement && name()=="forward")) {
		readNext();

		if (tokenType()==StartElement) {
			if (name()=="duration") {
				length = (int)((readElementText().toInt()/(float)divisions) * 256);
			} else if (name()=="voice") {
				voice = readElementText().toInt();
			} else if (name()=="staff") {
				staff = readElementText().toInt();
			}
		}
	}

	if (voice!=-1 && length!=-1) {
		CAVoice *v = addVoiceIfNeeded( partId, staff, voice );

		QList<CARest*> hiddenRests = CARest::composeRests( length, v->lastTimeEnd(), v );

		for (int i=0; i<hiddenRests.size(); i++) {
			v->append( hiddenRests[i] );
		}
	}
}
