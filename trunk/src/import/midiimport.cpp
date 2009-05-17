/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QTextStream>
#include <QRegExp>

#include <iostream> // DEBUG

#include "interface/mididevice.h"
#include "import/midiimport.h"
#include "score/note.h"
#include "score/playable.h"
#include "score/playablelength.h"
#include "score/clef.h"
#include "score/keysignature.h"
#include "score/timesignature.h"
#include "score/slur.h"
#include "score/sheet.h"
#include "score/document.h"

/*
	For the moment only for Linux, and parts of pmidi in use are still dependant on alsa, so:
*/
#if defined(__linux__) && defined(__LINUX_ALSASEQ__)
#include "import/pmidi/wrapper.h"
#endif

class CAMidiImportEvent {
public:
	CAMidiImportEvent( bool on, int channel, int pitch, int velocity, int time, int length );
	~CAMidiImportEvent();
	bool _on;
	int _channel;
	int _pitch;
	int _velocity;
	int _time;
	int _length;
	int _timeCorrection;
	int _lengthCorrection;
	int _nextTime;
};

CAMidiImportEvent::CAMidiImportEvent( bool on, int channel, int pitch, int velocity, int time, int length = 0){
	_on = on;
	_channel = channel;
	_pitch = pitch;
	_velocity = velocity;
	_time = time;
	_length = length;
	_timeCorrection = 0;
	_lengthCorrection = 0;
	_nextTime = time+length;
}

CAMidiImportEvent::~CAMidiImportEvent() {
}


/*!
	Delimiters which separate various music elements in LilyPond syntax. These are new lines, tabs, blanks etc.

	\sa nextElement(), parseNextElement()
*/
const QRegExp CAMidiImport::WHITESPACE_DELIMITERS = QRegExp("[\\s]");

/*!
	Delimiters which separate various music elements in LilyPond syntax, but are specific for LilyPond syntax.
	They are reported as its own element when parsing the next element.

	\sa nextElement(), parseNextElement()
*/
const QRegExp CAMidiImport::SYNTAX_DELIMITERS = QRegExp("[<>{}]");

/*!
	Combined WHITESPACE_DELIMITERS and SYNTAX_DELIMITERS.
*/
const QRegExp CAMidiImport::DELIMITERS =
	QRegExp(
		CAMidiImport::WHITESPACE_DELIMITERS.pattern().left(CAMidiImport::WHITESPACE_DELIMITERS.pattern().size()-1) +
		CAMidiImport::SYNTAX_DELIMITERS.pattern().mid(1)
	);


CAMidiImport::CAMidiImport( QTextStream *in )
 : CAImport(in) {
	initMidiImport();
	std::cout<<"          FIXME: jetzt in midiimport!"<<std::endl;
	for (int i=0;i<16; i++) {
		_allChannelsEvents << new QList<QList<CAMidiImportEvent*>*>;
		_allChannelsEvents[i]->append( new QList<CAMidiImportEvent*> );
		_allChannelsMediumPitch << 0;
	}
}

CAMidiImport::~CAMidiImport() {
}

void CAMidiImport::initMidiImport() {
	_curLine = _curChar = 0;
	_curSlur = 0; _curPhrasingSlur = 0;
	_document = 0;
	_microSecondsPerMidiQuarternote = 0;
}

void CAMidiImport::addError(QString description, int curLine, int curChar) {
	_errors << QString(QObject::tr("<i>Fatal error, line %1, char %2:</i><br>"))
	           .arg(curLine?curLine:_curLine)
	           .arg(curChar?curChar:_curChar)
	           + description + "<br>";
}

CADocument *CAMidiImport::importDocumentImpl() {
	_document = new CADocument();
	CASheet *s = importSheetImpl();
	_document->addSheet(s);

	return _document;
}


CASheet *CAMidiImport::importSheetImpl() {
	CASheet *sheet = new CASheet(tr("Midi imported sheet"), _document );
#if defined(__linux__) && defined(__LINUX_ALSASEQ__)
	return importSheetImplPmidiParser(sheet);
#else
	return importSheetImplOwnParser(sheet);
#endif
}

static int zaehler = 0;

CASheet *CAMidiImport::importSheetImplPmidiParser(CASheet *sheet) {
#if defined(__linux__) && defined(__LINUX_ALSASEQ__)
	QByteArray s;
	s.append(fileName());
	pmidi_open_midi_file( s.constData() );
	int voiceIndex;
	int res = PMIDI_STATUS_DUMMY;
	for (;res != PMIDI_STATUS_END;) {
		//if (zaehler++ >4170) break;
		res = pmidi_parse_midi_file();
		switch (res) {
		case PMIDI_STATUS_END:
		case PMIDI_STATUS_VERSION:
		case PMIDI_STATUS_TEXT:
		case PMIDI_STATUS_TIMESIG:
		case PMIDI_STATUS_TEMPO:
			std::cout<<" Tempo "<<pmidi_out.micro_tempo
				<<"  at "<<pmidi_out.time
				<<std::endl;
			break;
		case PMIDI_STATUS_NOTE:
			std::cout<<" at "<<pmidi_out.time
				<<"  chan "<<pmidi_out.chan
				<<" note "<<pmidi_out.note
				<<" vel "<<pmidi_out.vel
				<<" len "<<pmidi_out.length<<std::endl;
			// Scale music time properly
			pmidi_out.time = (pmidi_out.time *256)/pmidi_out.time_base;			// 24 is "Clocks" ?
			pmidi_out.length = (pmidi_out.length *256)/pmidi_out.time_base;
			
			// Get note to the right voice
			voiceIndex=0;
			for (voiceIndex=0;;) {
				if (_allChannelsEvents[pmidi_out.chan]->at(voiceIndex)->size() == 0 ||
					_allChannelsEvents[pmidi_out.chan]->at(voiceIndex)->last()->_nextTime <= pmidi_out.time) {
					// the note can be added
					_allChannelsEvents[pmidi_out.chan]->at(voiceIndex)->append( new CAMidiImportEvent( true,
							pmidi_out.chan, pmidi_out.note, pmidi_out.vel, pmidi_out.time, pmidi_out.length ));
					break;
				}
				// if another voice is needed create it
				if (++voiceIndex >= _allChannelsEvents[pmidi_out.chan]->size()) {
					_allChannelsEvents[pmidi_out.chan]->append( new QList<CAMidiImportEvent*> );
				}
				if (voiceIndex>30) break;
			}
			break;
		case PMIDI_STATUS_DUMMY:
		case PMIDI_STATUS_ROOT:
			std::cout<<" Midi-Format: "<<pmidi_out.format
				<<"  Tracks: "<<pmidi_out.tracks
				<<"  Time Base: "<<pmidi_out.time_base<<std::endl;
			break;
		case PMIDI_STATUS_CONTROL:
		case PMIDI_STATUS_PROGRAM:
		case PMIDI_STATUS_PITCH:
		case PMIDI_STATUS_PRESSURE:
		case PMIDI_STATUS_KEYTOUCH:
		case PMIDI_STATUS_SYSEX:
			break;
		case PMIDI_STATUS_KEYSIG:
			std::cout<<" Keys: "<<pmidi_out.key
				<<"  Minor: "<<pmidi_out.minor<<std::endl;
			break;
		case PMIDI_STATUS_SMPTEOFFS:
			std::cout<<"  Stunden "<<pmidi_out.hours
				<<" Minuten "<<pmidi_out.minutes
				<<" Sekunden "<<pmidi_out.seconds
				<<" Frames "<<pmidi_out.frames
				<<" Subframes "<<pmidi_out.subframes<<std::endl;
			break;
		}

		//std::cout<<p<<std::endl;

	}
#endif
	writeMidiFileEventsToScore_New( sheet );
	return sheet;
}

CASheet *CAMidiImport::importSheetImplOwnParser(CASheet *sheet) {

	QByteArray s;
	s.append(fileName());


	QString alles;
	stream()->setCodec("Latin-1");	// Binary files like midi files need all codecs to be switched off. This does it!?!?
	alles = stream()->readAll();
	std::cout<<"              FIXME: did a sheet export. In-File ist leer: "<<alles.size()<<std::endl;
	QByteArray peek;
	peek.append( alles );
	int z = 0;
	QByteArray head;
	int length;
	int midiFormatVersion;
	int midiTimeDivision;
	int numberOfTracks;
	int deltaTime;
	int sharps;
	int minor;
	int pitch;
	int velocity;
	int control;
	int controlValue;
	int program;
	int midiChannel;
	int combinedEvent;
	int event;
	int metaEvent;
	_dataIndex=0;
	_nextTrackIndex=0;
	int time = 0;
	_parseError = false;

	while (_dataIndex<peek.size() && !_parseError) {

		head.clear();
		head = getHead( &peek );
		std::cout<<"Head read"<<std::endl;

		if (head=="MThd") {

			length = getWord32( &peek );
			midiFormatVersion = getWord16( &peek );
			numberOfTracks = getWord16( &peek );
			midiTimeDivision = getWord16( &peek );
			std::cout<<"MThd.... Länge: "<<length<<" Format/Version: "<<midiFormatVersion
				<<" no. of Tracks: "<<numberOfTracks<<" MidiTimeDiv: "<<midiTimeDivision<<std::endl;

		} else if (head=="MTrk") {

			time = 0;	// FIXME: until we know better we restart time with every Track

			length = getWord32( &peek );
			std::cout<<" MTrk.... Länge: "<<length<<std::endl;

			if (length<0) {
				std::cout<<"  negative!"<<std::endl;
				int a = _dataIndex-10<0 ? 0 : _dataIndex-10;
				int b = _dataIndex+10>peek.size() ? peek.size() : _dataIndex+10;
				printQByteArray(peek.mid(a,b-a));
				printQByteArray(peek.mid(_dataIndex-4,4));
				return 0;
			}

			_nextTrackIndex = _dataIndex + length;

			while (_dataIndex < _nextTrackIndex && !_parseError) {
				deltaTime = getVariableLength( &peek );
				time += deltaTime;
				event = getByte( &peek );

				switch (event) {

				case CAMidiDevice::Midi_Ctl_Event:

					metaEvent = getByte( &peek );
					std::cout<<"  MetaEvent "<<metaEvent<<std::endl;

					switch (metaEvent) {
					case CAMidiDevice::Meta_Text:
						length = getVariableLength( &peek );
						std::cout<<// " len "<<length<<" vorher "<<_dataIndex<<
							"     "<<getString( &peek, length ).constData()<<::std::endl;
						//	" nachher "<<_dataIndex<<std::endl;
						break;
					case CAMidiDevice::Meta_Tempo:	// FIXME
						length = getByte( &peek );
						if (length != 3) _parseError = true;
						_microSecondsPerMidiQuarternote = getWord24( &peek );
						std::cout<<"    Tempo: "<<_microSecondsPerMidiQuarternote<<" usec per midi quarter"<<std::endl;
						break;

					case CAMidiDevice::Meta_InstrName:	// FIXME
					case CAMidiDevice::Meta_SeqTrkName: // 3
						length = getVariableLength( &peek );
						getString( &peek, length );
						printf("    metaEv: %x   ", metaEvent);
						for (int jj=0; jj<length; jj++ ) {
							printf(" %2x", peek.at(_dataIndex-length+jj) & 0x0ff);
						}
						printf("\n");
						break;
					case CAMidiDevice::Meta_SMPTEOffs:	// FIXME
						length = getVariableLength( &peek );
						if (length != 5) {
							std::cout<<"  wrong SMPTE Offset length"<<std::endl;
							_parseError = true;
							break;
						}
						printf("    metaEv: %x   SMPTE Offset:", metaEvent );
						for (int i=0;i<next;i++) {
							_smtpOffset[i] = getByte( &peek );
						printf(" %d", _smtpOffset[i]);
						}
						printf("\n");
						break;
					case CAMidiDevice::Meta_Keysig:
						length = getVariableLength( &peek );
						sharps = getByte( &peek );
						minor = getByte( &peek );	// four Bytes to be processed, todo
						std::cout<<"     Keysig "<<sharps<<" "<<minor<<std::endl;
						break;
					case CAMidiDevice::Meta_Timesig:
						length = getVariableLength( &peek );
						length = getWord32( &peek );	// four Bytes to be processed, todo
						std::cout<<"     Timesig "<<length<<" "<<minor<<std::endl;
						break;
					case CAMidiDevice::Meta_Track_End:
						getByte( &peek );
						break;

					default:	// here we process events that have command + midi channel combined in one byte
						std::cout<<"  unrecognized meta event "<<metaEvent<<" after event "<<event<<std::endl;
						return 0;
					}
					break;
				default:
					// printQByteArray( peek.mid(_dataIndex,_dataIndex+16<peek.size()?16:0));

					midiChannel = event & 0x0f;
					combinedEvent = event & 0x0f0;

					switch (combinedEvent) {
					case CAMidiDevice::Midi_Note_On:
						pitch = getByte( &peek );
						velocity = getByte( &peek );
						//std::cout<<"     note on "<<hex<<pitch<<" "<<hex<<velocity<<" at "<<time<<" ms"<<std::endl;
						printf("     note on %x %x at %d ms    kanal %d\n", pitch, velocity, time, midiChannel );
						noteOn( true, midiChannel, pitch, velocity, time );
						break;
					case CAMidiDevice::Midi_Note_Off:
						pitch = getByte( &peek );
						velocity = getByte( &peek );
						std::cout<<"     note off "<<pitch<<" "<<velocity<<" ch "<<midiChannel<<std::endl;
						noteOn( false, midiChannel, pitch, velocity, time );
						break;
					case CAMidiDevice::Midi_Prog_Change:
						program = getByte( &peek );
						std::cout<<"     prog change "<<program<<std::endl;
						break;
					case CAMidiDevice::Midi_Control_Chg:
						control = getByte( &peek );
						controlValue = getByte( &peek );
						std::cout<<"     control change "<<control<<" val "<<controlValue<<std::endl;
						break;
					case CAMidiDevice::Midi_Ctl_Sustain:	// where in midi spec?
						control = getByte( &peek );
						controlValue = getByte( &peek );
						std::cout<<"     control change "<<control<<" val "<<controlValue<<std::endl;
						_parseError = true;
						break;

					//case CAMidiDevice::MIDI_CTL_REVERB:  ;
					//case CAMidiDevice::MIDI_CTL_CHORUS:  ;
					//case CAMidiDevice::MIDI_CTL_PAN:  ;
					//case CAMidiDevice::MIDI_CTL_VOLUME:  ;
					//case CAMidiDevice::MIDI_CTL_SUSTAIN: ;

					default:	int a = _dataIndex-20 >=0 ? _dataIndex-20: 0;
								printQByteArray( peek.mid(a,_dataIndex-a+1));
								int b = _dataIndex+20 < peek.size() ? 20 : peek.size()-_dataIndex;
								printQByteArray( peek.mid(_dataIndex, b ));
								//std::cout<<"Hier, was ist zu tun?  Event: "<<hex<<int(event)<<" im File char "<<_dataIndex<<std::endl;
								printf("Hier, was ist zu tun?  Event: %x im File char %d\n", event, _dataIndex);
								_parseError = true;
					}
				}
			} // end of track elements
		} else { // end of track
			std::cout<<"Track Header not recognized"<<std::endl;
			int a = _dataIndex-10<0 ? 0 : _dataIndex-10;
			int b = _dataIndex+10>peek.size() ? peek.size() : _dataIndex+10;
			printQByteArray(peek.mid(a,b-a));
			return 0;
		}
	} // end of file

	combineMidiFileEvents();
	quantizeMidiFileEvents();
	exportNonChordsToOtherVoices();

	writeMidiFileEventsToScore( sheet );
	std::cout<<"------------------------------"<<std::endl;
/*
	for (int i=0;i<_events.size(); i++) {
		std::cout<<"......   "<<_events[i]->_on
						<<"  "<<_events[i]->_channel
						<<"  "<<_events[i]->_pitch
						<<"  "<<_events[i]->_velocity
						<<"  "<<_events[i]->_time
						<<" l "<<_events[i]->_length
		<<std::endl;
	}
*/


	return sheet;
}


void CAMidiImport::writeMidiFileEventsToScore_New( CASheet *sheet ) {

//	QList<CAPlayableLength> timeLengthToPlayableLengthList( int timeLength, bool longNotesFirst = true, int dotsLimit = 4 );
//	QList<CAPlayableLength> matchToBars( CAPlayableLength len, int timeStart, CABarline *lastBarline, CATimeSignature *ts, int dotsLimit = 4 );


	QString sheetName("imported");
	//CASheet *sheet = new CASheet( sheetName, _document );
	int numberOfStaffs = sheet->staffList().size();
	int staffIndex = 0;
	int voiceIndex;
	CAStaff *staff;
	CAVoice *voice;

	for (int chanIndex=0;chanIndex<16;chanIndex++) {
		int n=0;
		_allChannelsMediumPitch[chanIndex]=0;
		for (voiceIndex=0;voiceIndex<_allChannelsEvents[chanIndex]->size();voiceIndex++) {
			for (int i=0;i<_allChannelsEvents[chanIndex]->at(voiceIndex)->size();i++) {
				n++;
				_allChannelsMediumPitch[chanIndex] += _allChannelsEvents[chanIndex]->at(voiceIndex)->at(i)->_pitch;
			}
		}
		if (n>0) {
			_allChannelsMediumPitch[chanIndex]=_allChannelsMediumPitch[chanIndex]/n;
			std::cout<<"Channel "<<chanIndex<<" has "<<_allChannelsEvents[chanIndex]->size()<<" Voices, Medium-Pitch "
				<<_allChannelsMediumPitch[chanIndex]<<std::endl;
		}
	}

	// Quantization on sixtyfourth of time starts and lengths by zeroing the msbits
	for (int ch=0;ch<16;ch++) {

		for (voiceIndex=0;voiceIndex<_allChannelsEvents[ch]->size();voiceIndex++) {
			for (int i=0;i< _allChannelsEvents[ch]->at(voiceIndex)->size();i++) {
				_allChannelsEvents[ch]->at(voiceIndex)->at(i)->_time &= ~7;
				_allChannelsEvents[ch]->at(voiceIndex)->at(i)->_length &= ~7;
			}
		}
	}

	for (int ch=0;ch<16;ch++) {

		if (!_allChannelsEvents[ch]->size() || !_allChannelsEvents[ch]->first()->size())	/* staff or first voice empty */
			continue;

		if (staffIndex < numberOfStaffs) {
			staff = sheet->staffList().at(staffIndex);
			voice = staff->voiceList().first();
		} else {
			// create a new staff with 5 lines
			staff = new CAStaff( "", sheet, 5);
			sheet->addContext(staff);
		}
		for (int voiceIndex=0;voiceIndex<_allChannelsEvents[ch]->size();voiceIndex++) {
			// voiceName = QObject::tr("Voice%1").arg( voiceNumber );
			voice = new CAVoice( "", staff, CANote::StemNeutral, 1 );
			staff->addVoice( voice );

			if (voiceIndex==0) {
				CAMusElement *musElemClef;
				if (_allChannelsMediumPitch[ch] < 50) {
					musElemClef = new CAClef(CAClef::Bass, staff, 0, 0 );
				} else
					musElemClef = new CAClef(CAClef::Treble, staff, 0, -8 );
				if (_allChannelsMediumPitch[ch] < 65) {
				} else {
					musElemClef = new CAClef(CAClef::Treble, staff, 0, 0 );
				}
				//bool success = staff->voiceAt(0)->insert( 0 /* right */, musElemClef );
				bool success = staff->voiceList()[0]->insert( 0, musElemClef );
			
				CADiatonicKey dk = CADiatonicKey(
					pmidi_out.key, pmidi_out.minor ? CADiatonicKey::Minor : CADiatonicKey::Major );
				CAMusElement *musElemKeySig = new CAKeySignature( dk, staff, 0 );
				success = staff->voiceList()[0]->insert( 0 /* musElemClef */, musElemKeySig );

				CAMusElement *musElemTimeSig = new CATimeSignature( pmidi_out.top, pmidi_out.bottom, staff, 0 );
				success = staff->voiceList()[0]->insert( 0 /* musElemClef */, musElemTimeSig );
			}

			setCurVoice(voice);
			writeMidiChannelEventsToVoice_New( ch, voiceIndex, staff, voice );
			staff->synchronizeVoices();
		}

		staffIndex++;
	}
}


void CAMidiImport::writeMidiFileEventsToScore( CASheet *sheet ) {

//	QList<CAPlayableLength> timeLengthToPlayableLengthList( int timeLength, bool longNotesFirst = true, int dotsLimit = 4 );
//	QList<CAPlayableLength> matchToBars( CAPlayableLength len, int timeStart, CABarline *lastBarline, CATimeSignature *ts, int dotsLimit = 4 );


	QString sheetName("imported");
	//CASheet *sheet = new CASheet( sheetName, _document );
	int numberOfStaffs = sheet->staffList().size();
	int staffIndex = 0;
	CAStaff *staff;
	CAVoice *voice;

	for (int ch=0;ch<16;ch++) {

		if (!_allChannelsEvents[ch]->size())
			continue;

		if (staffIndex < numberOfStaffs) {
			staff = sheet->staffList().at(staffIndex);
			voice = staff->voiceList().first();
		} else {
			// create a new staff with 5 lines
			staff = new CAStaff( "", sheet, 5);
			sheet->addContext(staff);
			// voiceName = QObject::tr("Voice%1").arg( voiceNumber );
			voice = new CAVoice( "", staff, CANote::StemNeutral, 1 );
			staff->addVoice( voice );
		}

		setCurVoice(voice);
		writeMidiChannelEventsToVoice( ch, staff, voice );

		staffIndex++;
	}
}

void CAMidiImport::writeMidiChannelEventsToVoice_New( int channel, int voiceIndex, CAStaff *staff, CAVoice *voice ) {

	QList<CAMidiImportEvent*> *events = _allChannelsEvents[channel]->at(voiceIndex);
	CANote *note;
	CARest *rest;
	CANote *previousNote;	// for sluring
	QList<CAPlayableLength> timeLayout;
	CAPlayableLength dummy;
	int time = 0;			// current time in the loop, only increasing, for tracking notes and rests
	int length;
	int pitch;

	/* TODO:
	About tempo import: Beats (Quarters) per Minute = 60000000 / pmidi_out.micro_tempo.
	60000000 is usec per minute, micro_tempo is duration of a quarter in usec.
	*/

	// for debugging if (channel != 1 || voiceIndex != 0) return;
	std::cout<< "Channel "<<channel<<" VoiceIndex "<<voiceIndex<<" Pointer auf Voice: "<<voice<<std::endl;

	for (int i=0; i<events->size(); i++ ) {

		CABarline *b = static_cast<CABarline*>( voice->previousByType( CAMusElement::Barline,
			voice->lastMusElement()));
		CATimeSignature *ts = static_cast<CATimeSignature*>(
			voice->previousByType( CAMusElement::TimeSignature, voice->lastPlayableElt() ));
		QList<CAPlayableLength> lll;
		CAPlayableLength px;

		// check if we need to add rests
		length = events->at(i)->_time - time;
		if (length > 0) {
			lll << px.matchToBars( length, voice->lastTimeEnd(), b, ts );
			for (int j=0; j<lll.size(); j++) {
				rest = new CARest( CARest::Normal, lll[j], voice, 0, -1 );
				voice->append( rest, false );
			  	musElementFactory()->placeAutoBar( rest );
			}
			time += length;
		}
		// notes to be added
		b = static_cast<CABarline*>( voice->previousByType( CAMusElement::Barline,
			voice->lastMusElement()));
		length = events->at(i)->_length;
		pitch = events->at(i)->_pitch;
		if ( events->at(i)->_velocity > 0 && pitch > 0 && events->at(i)->_length > 0) {

		lll.clear();
		lll << px.matchToBars( length, voice->lastTimeEnd(), b, ts );

		//std::cout<< i<<":"<<length<<"   "<<lll.size()<<std::endl;
			previousNote = 0;
			for (int j=0; j<lll.size();j++) {
				//std::cout<< i<<" "<<j<<"    "<<time<<" "<<length<<std::endl;
				CADiatonicPitch diap = matchPitchToKey( voice, CAMidiDevice::midiPitchToDiatonicPitch(pitch) );
				//note = new CANote( CAMidiDevice::midiPitchToDiatonicPitch(pitch), lll[j], voice, -1 );
				note = new CANote( diap, lll[j], voice, -1 );
				// TODO note = new CANote( nonenharmonicPitch, tiemLayout[j], voice, -1 );
				voice->append( note, false );
			  	musElementFactory()->placeAutoBar( note );
				if (previousNote) {
					CASlur *slur = new CASlur( CASlur::TieType, CASlur::SlurPreferred, staff, previousNote, note );
					previousNote->setTieStart( slur );
					note->setTieEnd( slur );
				}
				previousNote = note;
			}
			time += length;
		}
	}
	return;
	// old variant:
	for (int i=0; i<events->size(); i++ ) {
//std::cout<<"Schleife 0 "<<i<<std::endl;
		pitch = events->at(i)->_pitch;
		if (events->at(i)->_on && events->at(i)->_velocity > 0 && pitch > 0 && events->at(i)->_length > 0) {
//std::cout<<"Schleife 1 "<<i<<std::endl;
			length = events->at(i)->_time - time;
			if ( length > 0 ) {

				CABarline *b = static_cast<CABarline*>( voice->previousByType( CAMusElement::Barline,
					voice->lastMusElement()));
				CATimeSignature *ts = static_cast<CATimeSignature*>(
					voice->previousByType( CAMusElement::TimeSignature, voice->lastPlayableElt() ));

				QList<CAPlayableLength> lll;
				CAPlayableLength px;
				//lll << px.matchToBars( musElementFactory()->playableLength(), voice->lastTimeEnd(), b, ts );

				timeLayout.clear();
				timeLayout << dummy.timeLengthToPlayableLengthList( length );
				for (int j=0; j<timeLayout.size();j++) {
					rest = new CARest( voiceIndex==0 ? CARest::Normal : CARest::Hidden, timeLayout[j], voice, 0, -1 );
					voice->append( rest, false );
				}
				time = events->at(i)->_time;
			}
			length = events->at(i)->_length;
			timeLayout.clear();
			timeLayout << dummy.timeLengthToPlayableLengthList( length );
			previousNote = 0;
			for (int j=0; j<timeLayout.size();j++) {
				note = new CANote( CAMidiDevice::midiPitchToDiatonicPitch(pitch), timeLayout[j], voice, -1 );
				// TODO note = new CANote( nonenharmonicPitch, tiemLayout[j], voice, -1 );
				voice->append( note, false );
				if (previousNote) {
					CASlur *slur = new CASlur( CASlur::TieType, CASlur::SlurPreferred, staff, previousNote, note );
					previousNote->setTieStart( slur );
					note->setTieEnd( slur );
				}
				previousNote = note;
			}
			time += events->at(i)->_length;
		}
	}
}


void CAMidiImport::writeMidiChannelEventsToVoice( int channel, CAStaff *staff, CAVoice *voice ) {

	QList<CAMidiImportEvent*> *events = _allChannelsEvents[channel]->first();
	CANote *note;
	CARest *rest;
	CANote *previousNote;	// for sluring
	QList<CAPlayableLength> timeLayout;
	CAPlayableLength dummy;
	int time = 0;			// current time in the loop, only increasing, for tracking notes and rests
	int length;
	int pitch;
	for (int i=0; i<events->size() && i<1000; i++ ) {	// FIXME: limit is for debugging only
//std::cout<<"Schleife 0 "<<i<<std::endl;
		pitch = events->at(i)->_pitch;
		if (events->at(i)->_on && events->at(i)->_velocity > 0 && pitch > 0 && events->at(i)->_length > 0) {
//std::cout<<"Schleife 1 "<<i<<std::endl;
			length = events->at(i)->_time - time;
			if ( length > 0 ) {
				timeLayout.clear();
				timeLayout << dummy.timeLengthToPlayableLengthList( length );
				for (int j=0; j<timeLayout.size();j++) {
					rest = new CARest( CARest::Normal, timeLayout[j], voice, 0, -1 );
					voice->append( rest, false );
				}
				time = events->at(i)->_time;
			}
			length = events->at(i)->_length;
			timeLayout.clear();
			timeLayout << dummy.timeLengthToPlayableLengthList( length );
			previousNote = 0;
			for (int j=0; j<timeLayout.size();j++) {
				note = new CANote( CAMidiDevice::midiPitchToDiatonicPitch(pitch), timeLayout[j], voice, -1 );
						// TODO: note = new CANote( nonenharmonicPitch, lll[i], voice, -1 );
				voice->append( note, false );
				if (previousNote) {
					CASlur *slur = new CASlur( CASlur::TieType, CASlur::SlurPreferred, staff, previousNote, note );
					previousNote->setTieStart( slur );
					note->setTieEnd( slur );
				}
				previousNote = note;
			}
			time += events->at(i)->_length;
		}
	}
}

/*!
	Combines the midi on/off events and reduces them to a note with a length.
	Note off events are, which can also be a note on with velocity zero, are thus eaten up and
	are invalidated by pitch -1.

	todo: rounding fine grained note lenghts, dispersing small rests

	todo: combine concurrent notes to chords or export concurrent notes to a next voice.
*/
void CAMidiImport::combineMidiFileEvents() {
	for (int ch=0;ch<_allChannelsEvents.size();ch++) {

		QList<CAMidiImportEvent*> *events = _allChannelsEvents[ch]->first();

		for (int i=0;i<events->size();i++) {
			if (events->at(i)->_on && events->at(i)->_velocity > 0 && events->at(i)->_pitch > 0) {
				int j = i+1;
				int pitch = events->at(i)->_pitch;
				while ( j < events->size() ) {

					if (events->at(j)->_pitch == pitch &&
							(!events->at(j)->_on || events->at(j)->_velocity == 0)) {
						events->at(i)->_length = events->at(j)->_time - events->at(i)->_time;
						events->at(j)->_pitch = -1;
						events->at(j)->_on = false;
						break;
					}
					j++;
				}
			}
		}
		// no we cleanup unpaired note on's
		for (int i=0;i<events->size();i++) {
			if (events->at(i)->_on && events->at(i)->_length == 0 ) {
						events->at(i)->_on = false;
						events->at(i)->_pitch = -1;
			}
		}
	}
}

/*!
	Quantisize the notes and rests, don't affect the duration of the music.
*/
void CAMidiImport::quantizeMidiFileEvents() {

	const int roundQuant = 32;

	for (int ch=0;ch<_allChannelsEvents.size();ch++) {

		QList<CAMidiImportEvent*> *events = _allChannelsEvents[ch]->first();

		int nLostNotes = 0;

		int prevTimeCorrection = 0;		// not yet in use
		int prevLengthCorrection = 0;

		for (int i=0;i<events->size();i++) {

			events->at(i)->_timeCorrection = events->at(i)->_lengthCorrection = 0;

			if (events->at(i)->_on && events->at(i)->_pitch > 0) {

				int time = events->at(i)->_time;
				int timeRounded = (time + roundQuant/2) / roundQuant;
				timeRounded *= roundQuant;
				events->at(i)->_time = timeRounded;
				events->at(i)->_timeCorrection = timeRounded - time;

				int length = events->at(i)->_length;
				int lenRounded = (length + roundQuant/2) / roundQuant;
				lenRounded *= roundQuant;
				events->at(i)->_length = lenRounded;
				events->at(i)->_lengthCorrection = lenRounded - length;
				if (!lenRounded) {
					events->at(i)->_on = false;
					events->at(i)->_pitch = -1;
					nLostNotes++;
				}
			}
			if (nLostNotes) {
				std::cout<<"Due to rounding "<<nLostNotes<<" Notes got lost in Midi Channel "<<i<<"."<<std::endl;
			}
		}
	}
}

/*!
	If not a chord move overlapping notes to other voices.

	The algorithm is this: For every note on event we look if there are follow up events that overlap.
	If there is a overlap the second note will be moved to a higher (index of the) voice,
	so high, where it can stay without overlap.
*/
void CAMidiImport::exportNonChordsToOtherVoices() {

	for (int ch=0;ch<_allChannelsEvents.size();ch++) {
		QList<CAMidiImportEvent*> *events = _allChannelsEvents[ch]->first();
		int erasedNote = 0;
		for (int i=0;i<events->size() -1 ;i++) {
			if (events->at(i)->_on) {
				int time = events->at(i)->_time;
				int timeEnd = time + events->at(i)->_length;
				int next = events->at(i+1)->_time;
std::cout<<"ch "<<ch<<" at "<<i<<" start "<<time<<" Ende "<<timeEnd<<" nächste "<<next<<std::endl;
				int j = i;
				while (true) {
					j++;
					if (j >= events->size())	// don't run out of the list
						break;
					if (!events->at(j)->_on)	// we look only on note on events
						continue;
					if (events->at(j)->_time >= timeEnd)
						break;
					// overlapping events are deleted
					events->at(j)->_on = false;		// todo: this note should be moved to another voice
					events->at(j)->_pitch = -1;
std::cout<<"xx "<<ch<<" at "<<j<<" start "<<events->at(j)->_time<<std::endl;
					erasedNote++;
				}
			}
		}
		std::cout<<"Erased "<<erasedNote<<" overlapping Notes on Channel "<<ch<<std::endl;
	}

}

void CAMidiImport::closeFile() {
	file()->close();
}

void CAMidiImport::noteOn( bool on, int channel, int pitch, int velocity, int time) {

	// for now we put everything in the first voice of the channel
	_allChannelsEvents[channel]->first()->append( new CAMidiImportEvent( on, channel, pitch, velocity, time ));
}

/*!
	Returns the first element in input stream ended with one of the delimiters and shorten input stream for the element.

	\todo Only one-character syntax delimiters are supported so far.
	\sa peekNextElement()
*/
const QString CAMidiImport::parseNextElement() {
	// find the first non-whitespace character
	int start = in().indexOf(QRegExp("\\S"));
	if (start==-1)
		start = 0;

	int i = in().indexOf(DELIMITERS, start);
	if (i==-1)
		i=in().size();

	QString ret;
	if (i==start) {
		// syntax delimiter only
		ret = in().mid(start,1); // \todo Support for syntax delimiters longer than 1 character
		in().remove(0, start+1);
	} else {
		// ordinary whitespace/syntax delimiter
		ret = in().mid(start, i-start);
		in().remove(0, i);
	}

	return ret;
}

/*!
	Returns the first element in input stream ended with one of the delimiters but don't shorten the stream.

	\sa parseNextElement()
*/
const QString CAMidiImport::peekNextElement() {
	// find the first non-whitespace character
	int start = in().indexOf(QRegExp("\\S"));
	if (start==-1)
		start = 0;

	int i = in().indexOf(DELIMITERS, start);
	if (i==-1)
		i=in().size();

	QString ret;
	if (i==start) {
		// syntax delimiter only
		ret = in().left(1); // \todo
	} else {
		// ordinary whitespace/syntax delimiter
		ret = in().mid(start, i-start);
	}

	return ret;
}

/*!
	Gathers a list of music elements with the given element's start time and returns the first music element in the
	gathered list with the same attributes.

	This method is usually called when voices have "shared" music elements (barlines, clefs etc.). However, in LilyPond
	syntax the music element can/should be present in all the voices. This function finds this shared music element, if
	it already exists.

	If the music element with the same properties exists, user should delete its own instance and add an already
	existing instance of the returned shared music element to the voice.

	\sa CAMusElement::compare()
*/
CAMusElement* CAMidiImport::findSharedElement(CAMusElement *elt) {
	if ( !curVoice() || !curVoice()->staff() )
		return 0;

	// gather a list of all the music elements of that type in the staff at that time
	QList<CAMusElement*> foundElts = curVoice()->staff()->getEltByType( elt->musElementType(), elt->timeStart() );

	// compare gathered music elements properties
	for (int i=0; i<foundElts.size(); i++)
		if (!foundElts[i]->compare(elt))             // element has exactly the same properties
			if (!curVoice()->musElementList().contains(foundElts[i])) // element isn't present in the voice yet
				return foundElts[i];

	return 0;
}

/*!
	Returns true, if the given LilyPond element is a note.

	\sa isRest()
*/
bool CAMidiImport::isNote(const QString elt) {
	return QString(elt[0]).contains(QRegExp("[a-g]"));
}

/*!
	Returns true, if the given LilyPond element is a rest.

	\sa isNote()
*/
bool CAMidiImport::isRest(const QString elt) {
	return (elt[0]=='r' || elt[0]=='s' || elt[0]=='R');
}

/*!
	Generates the note pitch and number of accidentals from the note written in LilyPond syntax.

	\sa playableLengthFromLilyPond()
*/
CADiatonicPitch CAMidiImport::relativePitchFromLilyPond(QString& constNName, CADiatonicPitch prevPitch, bool parse) {
	QString noteName = constNName;

	// determine pitch
	int curPitch = noteName[0].toLatin1() - 'a' + 5	// determine the 0-6 pitch from note name
	               - (prevPitch.noteName() % 7);
	while (curPitch<-3)	//normalize pitch - the max +/- interval is fourth
		curPitch+=7;
	while (curPitch>3)
		curPitch-=7;
	curPitch += prevPitch.noteName();

	// determine accidentals
	signed char curAccs = 0;
	while (noteName.indexOf("is") != -1) {
		curAccs++;
		noteName.remove(0, noteName.indexOf("is") + 2);
		if (parse)
			constNName.remove(0, constNName.indexOf("is") + 2);
	}
	while ((noteName.indexOf("es") != -1) || (noteName.indexOf("as") != -1)) {
		curAccs--;
		noteName.remove(0, ((noteName.indexOf("es")==-1) ? (noteName.indexOf("as")+2) : (noteName.indexOf("es")+2)) );
		if (parse)
			constNName.remove(0, ((constNName.indexOf("es")==-1) ? (constNName.indexOf("as")+2) : (constNName.indexOf("es")+2)) );
	}
	if (!curAccs && parse)
		constNName.remove(0, 1);

	// add octave up/down
	for (int i=0; i<noteName.size(); i++) {
		if (noteName[i]=='\'') {
			curPitch+=7;
			if (parse)
				constNName.remove(0,1);
		} else if (noteName[i]==',') {
			curPitch-=7;
			if (parse)
				constNName.remove(0,1);
		}
	}

	return CADiatonicPitch( curPitch, curAccs );
}

/*!
	Generates playable length and number of dots from the note/rest string in LilyPond syntax.
	If the playable element doesn't include length, { CAPlayable::CAPlayableLength::Undefined, 0 } is returned.
	This function also shortens the given string for the playable length, if \a parse is True.

	\sa relativePitchFromString()
*/
CAPlayableLength CAMidiImport::playableLengthFromLilyPond(QString& elt, bool parse) {
	CAPlayableLength ret;

	// index of the first number
	int start = elt.indexOf(QRegExp("[\\d]"));
	if (start == -1)  // no length written
		return ret;
	else {            // length written
		// count dots
		int d=0;
		int dStart;
		for (int i = dStart = elt.indexOf(".",start);
		     i!=-1 && i<elt.size() && elt[i]=='.';
		     i++, ret.setDotted( ret.dotted()+1 ));

		if (dStart == -1)
			dStart = elt.indexOf(QRegExp("[\\D]"), start);
		if (dStart == -1)
			dStart = elt.size();

		ret.setMusicLength( static_cast<CAPlayableLength::CAMusicLength>(elt.mid(start, dStart-start).toInt()) );
		if (parse)
			elt.remove(start, dStart-start+ret.dotted());
	}

	return ret;
}

/*!
	Genarates rest type from the LilyPond syntax for the given rest.
	This function also shortens the given string for the rest type, if \a parse is True.
*/
CARest::CARestType CAMidiImport::restTypeFromLilyPond( QString& elt, bool parse ) {
	CARest::CARestType t = CARest::Normal;

	if (elt[0]=='r' || elt[0]=='R')
		t = CARest::Normal;
	else
		t = CARest::Hidden;

	if (parse)
		elt.remove( 0, 1 );

	return t;
}

/*!
	Genarates clef type from the LilyPond syntax for the given clef from format "clefType".
*/
CAClef::CAPredefinedClefType CAMidiImport::predefinedClefTypeFromLilyPond( const QString constClef ) {
	// remove any quotes/double quotes
	QString clef(constClef);
	clef.remove(QRegExp("[\"']"));

	if ( clef.contains("treble") || clef.contains("violin") || clef.contains("G") ) return CAClef::Treble;
	if ( clef.contains("french") ) return CAClef::French;
	if ( clef.contains("bass") || clef.contains("F") ) return CAClef::Bass;
	if ( clef.contains("varbaritone") ) return CAClef::Varbaritone;
	if ( clef.contains("subbass") ) return CAClef::Subbass;
	if ( clef.contains("mezzosoprano") ) return CAClef::Mezzosoprano;
	if ( clef.contains("soprano") ) return CAClef::Soprano;
	if ( clef.contains("alto") ) return CAClef::Alto;
	if ( clef.contains("tenor") ) return CAClef::Tenor;
	if ( clef.contains("baritone") ) return CAClef::Baritone;
	if ( clef=="percussion" ) return CAClef::Percussion;
	if ( clef=="tab" ) return CAClef::Tablature;

	return CAClef::Treble;
}

/*!
	Returns the Canorus octava or whichever interval above or below the clef.
*/
int CAMidiImport::clefOffsetFromLilyPond( const QString constClef ) {
	// remove any quotes/double quotes
	QString clef(constClef);
	clef.remove(QRegExp("[\"']"));

	if ( !clef.contains("_") && !clef.contains("^") )
		return 0;

	int m;
	int idx = clef.indexOf("^");
	if (idx==-1) {
		idx = clef.indexOf("_");
		m=-1;
	} else
		m=1;

	return clef.right( clef.size()-(idx+1) ).toInt()*m;
}

/*!
	Returns the key signature gender from format \\genderType.
*/
CADiatonicKey::CAGender CAMidiImport::diatonicKeyGenderFromLilyPond(QString gender) {
	if (gender=="\\major")
		return CADiatonicKey::Major;
	else
		return CADiatonicKey::Minor;
}

/*!
	Returns the time signature beat and beats in beats/beat format.
*/
CAMidiImport::CATime CAMidiImport::timeSigFromLilyPond(QString timeSig) {
	int beats=0, beat=0;

	beats = timeSig.mid(0, timeSig.indexOf("/")).toInt();
	beat = timeSig.mid(timeSig.indexOf("/")+1).toInt();

	CATime time = { beats, beat };
	return time;
}

/*!
	Genarates barline type from the LilyPond syntax for the given barline from format "barlineType".
*/
CABarline::CABarlineType CAMidiImport::barlineTypeFromLilyPond(QString constBarline) {
	// remove any quotes/double quotes
	QString barline(constBarline);
	barline.remove(QRegExp("[\"']"));

	if (barline=="|") return CABarline::Single; else
	if (barline=="||") return CABarline::Double; else
	if (barline=="|.") return CABarline::End; else
	if (barline=="|:") return CABarline::RepeatOpen; else
	if (barline==":|") return CABarline::RepeatClose; else
	if (barline==":|:") return CABarline::RepeatCloseOpen; else
	if (barline==":") return CABarline::Dotted; else
	return CABarline::Undefined;
}

const QString CAMidiImport::readableStatus() {
	switch (status()) {
	case 0:
		return tr("Ready");
	case 1:
		return tr("Importing...");
	case -1:
		return tr("Error while importing!\nLine %1:%2.").arg(curLine()).arg(curChar());
	}
}


void CAMidiImport::printQByteArray( QByteArray x )
{
    for (int i=0; i<x.size(); i++ ) {
        printf( " %02x", 0x0ff & x.at(i));
    }
    printf( "\n");
}


QByteArray CAMidiImport::getHead(QByteArray *x) {
	QByteArray y;
	if (_dataIndex<x->size()-3) {
		y = x->mid(_dataIndex,4);
		_dataIndex += 4;
	} else {
		_parseError = true;
	}
	return y;
}

int CAMidiImport::getWord32(QByteArray *x) {
	unsigned int y;
	y = getWord16( x )<<16;
	y |= getWord16( x );
	return y;
}

int CAMidiImport::getWord24(QByteArray *x) {

	int y = getWord16( x )<<8;
	y |= getByte( x );
	return y;
}

int CAMidiImport::getWord16(QByteArray *x) {

	int y = 0;
	if (_dataIndex<x->size()-1) {
		y = ((x->at(_dataIndex++)) & 0x0ff)<<8;
		y |= (x->at(_dataIndex++)) & 0x0ff;
	} else {
		_parseError = true;
	}
	return y;
}

int CAMidiImport::getByte(QByteArray *x) {

	int b = 0;
	if (_dataIndex<x->size()) {
		b = x->at(_dataIndex++);
		b &= 0x0ff;
	} else {
		_parseError = true;
	}
	return b;
}

int CAMidiImport::getVariableLength(QByteArray *x) {

	int byte;
	unsigned int y = 0;

	int startIndex = _dataIndex;
	bool parsable = false;
	bool next = _dataIndex < x->size();
	_parseError |= not next;	// not shure if this covers all illegal lengths.
	while (_dataIndex < x->size() && next ) {
		byte = x->at(_dataIndex++) & 0x0ff;
		y = (y << 7) | (byte & 0x7f);
		next = (byte & 0x80) ? true : false;
		parsable = false;
	}
	return y;
}

QByteArray CAMidiImport::getString(QByteArray *x, int len) {
	QByteArray y;
	if (_dataIndex+len < x->size()){
		y = x->mid(_dataIndex,len);
		_dataIndex += len;
	} else {
		_parseError = true;
	}
	return y;
}


/*!
	This function is a duplicat in CAKeybdInput! Should be moved to CADiatonicPitch an be reused.

	This function looks up the current key signiture. Then it computes the proper accidentials
	for the note.

	This function should be somewhere else, maybe in \a CADiatonicPitch ?
*/
CADiatonicPitch CAMidiImport::matchPitchToKey( CAVoice* voice, CADiatonicPitch p ) {

	// Default actual Key Signature is C
	_actualKeySignature = CADiatonicPitch ( CADiatonicPitch::C );

	int i;
	for(i=0;i<7;i++) _actualKeySignatureAccs[i] = 0;
	_actualKeyAccidentalsSum = 0;

	// Trace which Key Signature might be in effect.
	// We make a local copy for later optimisation by only updating at a non
	// linear input
	QList<CAMusElement*> keyList = voice->getPreviousByType(
							CAMusElement::KeySignature, voice->lastTimeEnd());
	if (keyList.size()) {
		// set the note name and its accidental and the accidentals of the scale
		CAKeySignature* effSig = (CAKeySignature*) keyList.last();
		_actualKeySignature = effSig->diatonicKey().diatonicPitch();
		_actualKeyAccidentalsSum = 0;
		for(i=0;i<7;i++) {
			_actualKeySignatureAccs[i] = (effSig->accidentals())[i];
			_actualKeyAccidentalsSum += _actualKeySignatureAccs[i];
		}
	}

	// f and s (flat/sharp) are enharmonic pitches for p
	CADiatonicPitch f = p + CAInterval( CAInterval::Diminished, CAInterval::Second );
	CADiatonicPitch s = p - CAInterval( CAInterval::Diminished, CAInterval::Second );

	// If the pitch appears in the keys scale we are done
	if (p.accs() == _actualKeySignatureAccs[p.noteName()%7] ) {
		return p;
	}
	// When the key is with flats we don't want sharps
	if (_actualKeyAccidentalsSum < 0) {
		if (f.accs() == _actualKeySignatureAccs[f.noteName()%7] )
			return f;
 		if (p.accs() > 0)
			return f;
	}
	// When the key is with sharps we don't want flats
	if (_actualKeyAccidentalsSum > 0) {
		if (s.accs() == _actualKeySignatureAccs[s.noteName()%7] ) {
			return s;
		}
 		if (p.accs() < 0) {
			return s;
		}
	}
	return p;
}



