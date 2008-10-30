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
#include "core/note.h"
#include "core/playable.h"
#include "core/playablelength.h"
#include "core/slur.h"
#include "core/sheet.h"
#include "core/document.h"

class CAMidiImportEvent {
public:
	CAMidiImportEvent( bool on, int channel, int pitch, int velocity, int time );
	~CAMidiImportEvent();
	bool _on;
	int _channel;
	int _pitch;
	int _velocity;
	int _time;
	int _length;
};

CAMidiImportEvent::CAMidiImportEvent( bool on, int channel, int pitch, int velocity, int time){
	_on = on;
	_channel = channel;
	_pitch = pitch;
	_velocity = velocity;
	_time = time;
	_length = 0;
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

CAMidiImport::CAMidiImport( const QString in )
 : CAImport(in) {
	initMidiImport();
}

CAMidiImport::CAMidiImport( QTextStream *in )
 : CAImport(in) {
	initMidiImport();	
	std::cout<<"          FIXME: jetzt in midiimport deprec !"<<std::endl;
}

CAMidiImport::CAMidiImport( CADocument *document, QTextStream *in )
 : CAImport(in) {
	initMidiImport();	
	std::cout<<"          FIXME: jetzt in midiimport!"<<std::endl;
	_document = document;
}

CAMidiImport::~CAMidiImport() {
}

void CAMidiImport::initMidiImport() {
	_curLine = _curChar = 0;
	_curSlur = 0; _curPhrasingSlur = 0;
	_templateVoice = 0;
	_document = 0;
	_microSecondsPerMidiQuarternote = 0;
}

void CAMidiImport::addError(QString description, int curLine, int curChar) {
	_errors << QString(QObject::tr("<i>Fatal error, line %1, char %2:</i><br>"))
	           .arg(curLine?curLine:_curLine)
	           .arg(curChar?curChar:_curChar)
	           + description + "<br>";
}

CASheet *CAMidiImport::importSheetImpl() {
	QString alles;
	(*stream()).setCodec("Latin-1");	// Binary files like midi files need all codecs to be switched off. This does it!?!?
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

	for (int i=0;i<_events.size(); i++) {
		std::cout<<"......   "<<_events[i]->_on
						<<"  "<<_events[i]->_channel
						<<"  "<<_events[i]->_pitch
						<<"  "<<_events[i]->_velocity
						<<"  "<<_events[i]->_time
						<<" l "<<_events[i]->_length
		<<std::endl;
	}
	combineMidiFileEvents();
	CASheet *sheet = _document->sheetList().first();
	writeMidiFileEventsToScore( sheet );
	std::cout<<"------------------------------"<<std::endl;
	for (int i=0;i<_events.size(); i++) {
		std::cout<<"......   "<<_events[i]->_on
						<<"  "<<_events[i]->_channel
						<<"  "<<_events[i]->_pitch
						<<"  "<<_events[i]->_velocity
						<<"  "<<_events[i]->_time
						<<" l "<<_events[i]->_length
		<<std::endl;
	}


	return sheet;
}


void CAMidiImport::writeMidiFileEventsToScore( CASheet *sheet ) {

//	QList<CAPlayableLength> timeLengthToPlayableLengthList( int timeLength, bool longNotesFirst = true, int dotsLimit = 4 );
//	QList<CAPlayableLength> matchToBars( CAPlayableLength len, int timeStart, CABarline *lastBarline, CATimeSignature *ts, int dotsLimit = 4 );


	QString sheetName("imported");
	//CASheet *sheet = new CASheet( sheetName, _document );
	CAStaff *staff = sheet->staffList().first();
	CAVoice *voice = staff->voiceList().first();
	
	if (templateVoice())
		voice->cloneVoiceProperties( templateVoice() );
	
	setCurVoice(voice);

	CANote *note;
	CARest *rest;
	CANote *previousNote;	// for sluring
	QList<CAPlayableLength> timeLayout;
	CAPlayableLength dummy;
	int time = 0;			// current time in the loop, only increasing, for tracking notes and rests
	int length;
	int pitch;
	for (int i=0; i<_events.size(); i++ ) {
std::cout<<"Schleife 0 "<<i<<std::endl;
		pitch = _events[i]->_pitch;
		if (_events[i]->_on && _events[i]->_velocity > 0 && pitch > 0 && _events[i]->_length > 0) {
std::cout<<"Schleife 1 "<<i<<std::endl;
			length = _events[i]->_time - time;
			if ( length > 0 ) {
				timeLayout.clear();	
				timeLayout << dummy.timeLengthToPlayableLengthList( length );
				for (int j=0; j<timeLayout.size();j++) {
					rest = new CARest( CARest::Normal, timeLayout[j], voice, 0, -1 );
					voice->append( rest, false );
				}
				time = _events[i]->_time;
			}
			length = _events[i]->_length;
			timeLayout.clear();	
			timeLayout << dummy.timeLengthToPlayableLengthList( length );
			for (int j=0; j<timeLayout.size();j++) {
				note = new CANote( CAMidiDevice::midiPitchToDiatonicPitch(pitch), timeLayout[j], voice, -1 );
						// TODO: note = new CANote( nonenharmonicPitch, lll[i], voice, -1 );
				voice->append( note, false );
			}
			time += _events[i]->_length;
		}
	}
	std::cout<<" DEBUG "<<_document<<sheet<<std::endl;
}


void CAMidiImport::combineMidiFileEvents() {
	for (int i=0;i<_events.size();i++) {
		if (_events[i]->_on && _events[i]->_velocity > 0 && _events[i]->_pitch > 0) {
			int j = i+1;
			int pitch = _events[i]->_pitch;
			while ( j < _events.size() ) {
				
				if (_events[j]->_pitch == pitch &&
						(!_events[j]->_on || _events[j]->_velocity == 0)) {
					_events[i]->_length = _events[j]->_time - _events[i]->_time;
					_events[j]->_pitch = -1;
					break;
				}
				j++;
			}
		}
	}
}

void CAMidiImport::closeFile() {
	file()->close();
}

void CAMidiImport::noteOn( bool on, int channel, int pitch, int velocity, int time) {

	_events << new CAMidiImportEvent( on, channel, pitch, velocity, time );
}


CAVoice *CAMidiImport::importVoiceImpl() {
	CAVoice *voice = new CAVoice( "", 0 );
	
	if (templateVoice())
		voice->cloneVoiceProperties( templateVoice() );
	
	setCurVoice(voice);
	CADiatonicPitch prevPitch( 21, 0 );
	CAPlayableLength prevLength( CAPlayableLength::Quarter, 0 );
	bool chordCreated=false;
	bool changed=false;
	
	for (QString curElt = parseNextElement();
	     (!in().isEmpty());
	     curElt = ((curElt.size() && changed)?curElt:parseNextElement())) { // go to next element, if current one is empty or not changed
	    changed=true; // changed is default to true and false, if none of if clauses were found
		if (curElt.startsWith("\\relative")) {
			// initial \relative notePitch
			QString notePitch = parseNextElement();
			if (!isNote(notePitch)) {
				addError("\\relative doesn't include pitch.");
				continue;
			}
			
			prevPitch = relativePitchFromLilyPond(notePitch, 21);
			curElt.remove(0,9);
		} else
		if (curElt.startsWith("{")) {
			// start of the voice
			pushDepth(Voice);
			curElt.remove(0,1);			
		} else
		if (curElt.startsWith("}")) {
			// end of the voice
			popDepth();
			curElt.remove(0,1);
		} else
		if (curElt.startsWith("<")) {
			// start of the chord
			pushDepth(Chord);
			curElt.remove(0,1);
		} else
		if (curElt.startsWith(">")) {
			// end of the chord
			popDepth();
			chordCreated=false;
			if ( curVoice()->lastMusElement()->musElementType()==CAMusElement::Note ) {
				prevPitch = static_cast<CANote*>(curVoice()->lastMusElement())->getChord().at(0)->diatonicPitch();
			} else {
				addError(QString("Chord should be finished with a note."));
			}
			curElt.remove(0,1);
		} else
		if (curElt.startsWith("~")) {
			if ( curVoice()->lastMusElement()->musElementType()==CAMusElement::Note ) {
				CANote *note = static_cast<CANote*>(curVoice()->lastMusElement());
				note->setTieStart(
					new CASlur( CASlur::TieType, CASlur::SlurPreferred, note->staff(), note, 0 )
				);
			} else {
				addError(QString("Tie symbol must be right after the note and not %1. Tie ignored.").arg(CAMusElement::musElementTypeToString(curVoice()->lastMusElement()->musElementType())));
			}
			curElt.remove(0,1);
		} else
		if (curElt.startsWith("(")) {
			if ( curVoice()->lastMusElement()->musElementType()==CAMusElement::Note ) {
				CANote *note = static_cast<CANote*>(curVoice()->lastMusElement())->getChord().at(0);
				_curSlur = new CASlur( CASlur::SlurType, CASlur::SlurPreferred, note->staff(), note, 0 );
				note->setSlurStart(_curSlur);
			} else {
				addError(QString("Slur symbol must be right after the note and not %1. Slur ignored.").arg(CAMusElement::musElementTypeToString(curVoice()->lastMusElement()->musElementType())));
			}
			curElt.remove(0,1);
		} else
		if ( curElt.startsWith(")") && _curSlur ) {
			if ( curVoice()->lastMusElement()->musElementType()==CAMusElement::Note ) {
				CANote *note = static_cast<CANote*>(curVoice()->lastMusElement())->getChord().at(0);
				note->setSlurEnd(_curSlur);
				_curSlur->setNoteEnd(note);
				_curSlur=0;
			} else {
				addError(QString("Slur symbol must be right after the note and not %1. Slur ignored.").arg(CAMusElement::musElementTypeToString(curVoice()->lastMusElement()->musElementType())));
			}
			curElt.remove(0,1);
		} else
		if (curElt.startsWith("\\(")) {
			if ( curVoice()->lastMusElement()->musElementType()==CAMusElement::Note ) {
				CANote *note = static_cast<CANote*>(curVoice()->lastMusElement())->getChord().at(0);
				_curPhrasingSlur = new CASlur( CASlur::PhrasingSlurType, CASlur::SlurPreferred, note->staff(), note, 0 );
				note->setPhrasingSlurStart(_curPhrasingSlur);
			} else {
				addError(QString("Phrasing slur symbol must be right after the note and not %1. Phrasing slur ignored.").arg(CAMusElement::musElementTypeToString(curVoice()->lastMusElement()->musElementType())));
			}
			curElt.remove(0,2);
		} else
		if ( curElt.startsWith("\\)") && _curPhrasingSlur ) {
			if ( curVoice()->lastMusElement()->musElementType()==CAMusElement::Note ) {
				CANote *note = static_cast<CANote*>(curVoice()->lastMusElement())->getChord().at(0);
				note->setPhrasingSlurEnd(_curPhrasingSlur);
				_curPhrasingSlur->setNoteEnd(note);
				_curPhrasingSlur=0;
			} else {
				addError(QString("Phrasing slur symbol must be right after the note and not %1. Phrasing slur ignored.").arg(CAMusElement::musElementTypeToString(curVoice()->lastMusElement()->musElementType())));
			}
			curElt.remove(0,2);
		} else
		if (isNote(curElt)) {
			// CANote
			prevPitch = relativePitchFromLilyPond(curElt, prevPitch, true);
			CAPlayableLength length = playableLengthFromLilyPond(curElt, true);
			if (length.musicLength()!=CAPlayableLength::Undefined) // length may not be set
				prevLength = length;
			
			CANote *note;
			if (curDepth()!=Chord || !chordCreated) {
				// the note is not part of the chord or is the first note in the chord
				note = new CANote( prevPitch, prevLength, curVoice(), curVoice()->lastTimeEnd() );
				if (curDepth()==Chord)
					chordCreated = true;
				curVoice()->append( note, false );
			} else {
				// the note is part of the already built chord
				note = new CANote( prevPitch, prevLength, curVoice(), curVoice()->lastTimeStart() );
				curVoice()->append( note, true );
			}
			
			note->updateTies(); // close any opened ties if present
		} else
		if (isRest(curElt)) {
			// CARest
			CARest::CARestType type = restTypeFromLilyPond(curElt, true);
			CAPlayableLength length = playableLengthFromLilyPond(curElt, true);
			if (length.musicLength()!=CAPlayableLength::Undefined) // length may not be set
				prevLength = length;
						
			curVoice()->append( new CARest( type, prevLength, curVoice(), curVoice()->lastTimeEnd() ) );
		} else
		if (curElt.startsWith("|")) {
			// CABarline::Single
			// lookup an element with the same type at the same time
			CABarline *bar = new CABarline(CABarline::Single, curVoice()->staff(), curVoice()->lastTimeEnd());
			CABarline *sharedBar = static_cast<CABarline*>(findSharedElement(bar));

			if (!sharedBar) {
				curVoice()->append( bar );
			} else {
				curVoice()->append( sharedBar );
				delete bar;
			}
			curElt.remove(0,1);
		} else
		if (curElt.startsWith("\\bar")) {
			// CABarline
			QString typeString = peekNextElement();
			CABarline::CABarlineType type = barlineTypeFromLilyPond(peekNextElement());
			
			if (type==CABarline::Undefined) {
				addError(QString("Error while parsing barline type. Barline type %1 unknown.").arg(typeString));
			}
			
			// remove clef type from the input
			parseNextElement();
			
			// lookup an element with the same type at the same time
			CABarline *bar = new CABarline(type, curVoice()->staff(), curVoice()->lastTimeEnd());
			CABarline *sharedBar = static_cast<CABarline*>(findSharedElement(bar));

			if (!sharedBar) {
				curVoice()->append( bar );
			} else {
				curVoice()->append( sharedBar );
				delete bar;
			}
			curElt.remove(0,4);
		} else
		if (curElt.startsWith("\\clef")) {
			// CAClef
			QString typeString = peekNextElement();
			CAClef::CAPredefinedClefType type = predefinedClefTypeFromLilyPond( peekNextElement() );
			int offset = clefOffsetFromLilyPond( peekNextElement() );
			
			if (type==CAClef::Undefined) {
				addError(QString("Error while parsing clef type. Clef type %1 unknown.").arg(typeString));
			}
			
			// remove clef type from the input
			parseNextElement();
			
			CAClef *clef = new CAClef( type, curVoice()->staff(), curVoice()->lastTimeEnd(), offset );
			CAClef *sharedClef = static_cast<CAClef*>(findSharedElement(clef));
			
			if (!sharedClef) {
				curVoice()->append( clef );
			} else {
				curVoice()->append( sharedClef );
				delete clef;
			}
			curElt.remove(0,5);
		} else
		if (curElt=="\\key") {
			// CAKeySignature
			// pitch
			QString keyString = peekNextElement();
			if (!isNote(keyString)) {
				addError(QString("Error while parsing key signature. Key pitch %1 unknown.").arg(keyString));
				continue;
			}
			parseNextElement(); // remove pitch
			
			// gender
			QString genderString = peekNextElement();
			CADiatonicKey::CAGender gender = diatonicKeyGenderFromLilyPond(genderString);
			parseNextElement();
			
			CAKeySignature *keySig = new CAKeySignature( CADiatonicKey(relativePitchFromLilyPond(keyString, CADiatonicPitch(3) ), gender), curVoice()->staff(), curVoice()->lastTimeEnd() );
			CAKeySignature *sharedKeySig = static_cast<CAKeySignature*>(findSharedElement(keySig));
			
			if (!sharedKeySig) {
				curVoice()->append( keySig );
			} else {
				curVoice()->append( sharedKeySig );
				delete keySig;
			}
			curElt.remove(0,4);
		} else
		if (curElt.startsWith("\\time")) {
			// CATimeSignature
			QString timeString = peekNextElement();
			// time signature should have beats/beat format
			if (timeString.indexOf(QRegExp("\\d+/\\d+")) == -1) {
				addError(QString("Invalid time signature beats format %1. Beat and number of beats should be written <beats>/<beat>.").arg(timeString));
				continue;
			}
			CATime time = timeSigFromLilyPond(timeString);
			parseNextElement();
			
			CATimeSignature *timeSig = new CATimeSignature(time.beats, time.beat, curVoice()->staff(), curVoice()->lastTimeEnd());
			CATimeSignature *sharedTimeSig = static_cast<CATimeSignature*>(findSharedElement(timeSig));
			
			if (!sharedTimeSig) {
				curVoice()->append( timeSig );
			} else {
				curVoice()->append( sharedTimeSig );
				delete timeSig;
			}
			curElt.remove(0,5);
		} else
			changed=false;
	}
	
	return voice;
}

CALyricsContext *CAMidiImport::importLyricsContextImpl() {
	CALyricsContext *lc = new CALyricsContext( "", 1, static_cast<CASheet*>(0) );
	
	CASyllable *lastSyllable = 0;
	int timeSDummy=0; // dummy timestart to keep the order of inserted syllables. Real timeStarts are sets when repositSyllables() is called
	for (QString curElt = parseNextElement(); (!in().isEmpty() || !curElt.isEmpty() ); curElt = parseNextElement(), timeSDummy++) {
		QString text = curElt;
		if (curElt == "_")
			text = "";
		
		if (lastSyllable && text=="--") {
			lastSyllable->setHyphenStart(true);
		} else
		if (lastSyllable && text=="__") {
			lastSyllable->setMelismaStart(true);
		} else {
			lc->addSyllable( lastSyllable = new CASyllable( text, false, false, lc, timeSDummy, 0 ) );
		}
	}
	lc->repositSyllables(); // sets syllables timeStarts and timeLengths
	
	return lc;
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
			if (!curVoice()->contains(foundElts[i])) // element isn't present in the voice yet
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
	Generates playable lentgth and number of dots from the note/rest string in LilyPond syntax.
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


