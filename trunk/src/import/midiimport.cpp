/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QTextStream>
#include <QRegExp>
#include <QFileInfo>

#include <iostream> // DEBUG
#include <iomanip>

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
//#if defined(__linux__) && defined(__LINUX_ALSASEQ__)
#include "import/pmidi/wrapper.h"
//#endif

class CAMidiImportEvent {
public:
	CAMidiImportEvent( bool on, int channel, int pitch, int velocity, int time, int length, int tempo, int program );
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
	int _tempo;		// beats per minute
	int _top;
	int _bottom;
	int _program;
};

CAMidiImportEvent::CAMidiImportEvent( bool on, int channel, int pitch, int velocity, int time, int length = 0, int tempo = 120, int program = 0 ) {
	_on = on;
	_channel = channel;
	_pitch = pitch;
	_velocity = velocity;
	_time = time;
	_length = length;
	_timeCorrection = 0;
	_lengthCorrection = 0;
	_nextTime = time+length;
	_tempo = tempo;
	_program = program;
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
	sheet = importSheetImplPmidiParser(sheet);
	// Show filename as sheet name. The tr() string above should only be changed after a release.
	QFileInfo fi(fileName());
	sheet->setName(fi.baseName());
	return sheet;
}


CASheet *CAMidiImport::importSheetImplPmidiParser(CASheet *sheet) {
//#if defined(__linux__) && defined(__LINUX_ALSASEQ__)
	QByteArray s;
	s.append(fileName());
	pmidi_open_midi_file( s.constData() );
	int voiceIndex;
	int res = PMIDI_STATUS_DUMMY;
	const int quarterLength = CAPlayableLength::playableLengthToTimeLength( CAPlayableLength::Quarter );
	int programCache[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	for (;res != PMIDI_STATUS_END;) {

		res = pmidi_parse_midi_file();

		switch (res) {
		case PMIDI_STATUS_END:
		case PMIDI_STATUS_VERSION:
		case PMIDI_STATUS_TEXT:
			break;
		case PMIDI_STATUS_TIMESIG:
			std::cout<<" Timesig "<<pmidi_out.top
					<<"/"<<pmidi_out.bottom
					<<" at "<<pmidi_out.time
					<<std::endl;
			// Scale music time properly
			pmidi_out.time = (pmidi_out.time*quarterLength)/pmidi_out.time_base;

			// We build the list (vector) of time signatures. If the occurence in time is the same last one wins.
			if (!_allChannelsTimeSignatures.size() || _allChannelsTimeSignatures[_allChannelsTimeSignatures.size()-1]->_time != pmidi_out.time) {
				_allChannelsTimeSignatures << new CAMidiImportEvent( true, 0, 0, 0, pmidi_out.time, 0, 0 );
				_allChannelsTimeSignatures[_allChannelsTimeSignatures.size()-1]->_top = pmidi_out.top;
				_allChannelsTimeSignatures[_allChannelsTimeSignatures.size()-1]->_bottom = pmidi_out.bottom;
			} else {
				// overwrite the last one with new values
				_allChannelsTimeSignatures[_allChannelsTimeSignatures.size()-1]->_top = pmidi_out.top;
				_allChannelsTimeSignatures[_allChannelsTimeSignatures.size()-1]->_bottom = pmidi_out.bottom;
			}
			break;
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
			pmidi_out.time = (pmidi_out.time*quarterLength)/pmidi_out.time_base;
			pmidi_out.length = (pmidi_out.length*quarterLength)/pmidi_out.time_base;

			// Deal with unfinished notes. This is a note that get's keyed when the old same pitch note not yet expired.
			// Pmidi does a printf message with those, don't know yet how it handles then.
			// We do shorten them to the current time. This helps in many cases.
			for (voiceIndex=0;voiceIndex<_allChannelsEvents[pmidi_out.chan]->size();voiceIndex++) {
				bool lookedBackEnough = false;
				for (int i=_allChannelsEvents[pmidi_out.chan]->at(voiceIndex)->size()-1;i>=0;i--) {
					if (pmidi_out.note == _allChannelsEvents[pmidi_out.chan]->at(voiceIndex)->at(i)->_pitch) {
						if (pmidi_out.time < _allChannelsEvents[pmidi_out.chan]->at(voiceIndex)->at(i)->_nextTime) {
							_allChannelsEvents[pmidi_out.chan]->at(voiceIndex)->at(i)->_length = pmidi_out.time -
							_allChannelsEvents[pmidi_out.chan]->at(voiceIndex)->at(i)->_time;
							_allChannelsEvents[pmidi_out.chan]->at(voiceIndex)->at(i)->_nextTime = pmidi_out.time;
							// this would make them very short
							//_allChannelsEvents[pmidi_out.chan]->at(voiceIndex)->at(i)->_length = 8;
							//_allChannelsEvents[pmidi_out.chan]->at(voiceIndex)->at(i)->_nextTime = 
							//	_allChannelsEvents[pmidi_out.chan]->at(voiceIndex)->at(i)->_time + 8;
						}
						lookedBackEnough = true;
						break;
					}
				}
				if (lookedBackEnough) break;
			}
			
			// Get note to the right voice
			for (voiceIndex=0;voiceIndex<30;voiceIndex++) {		// we can't imagine that so many voices ar needed in any case so let's put a limit
				// if another voice is needed and not yet there we create it
				if (voiceIndex >= _allChannelsEvents[pmidi_out.chan]->size()) {
					_allChannelsEvents[pmidi_out.chan]->append( new QList<CAMidiImportEvent*> );
				}
				if (_allChannelsEvents[pmidi_out.chan]->at(voiceIndex)->size() == 0 ||
					_allChannelsEvents[pmidi_out.chan]->at(voiceIndex)->last()->_nextTime <= pmidi_out.time) {
					// the note can be added
					_allChannelsEvents[pmidi_out.chan]->at(voiceIndex)->append( new CAMidiImportEvent( true,
							pmidi_out.chan, pmidi_out.note, pmidi_out.vel, pmidi_out.time, pmidi_out.length,
							60000000/pmidi_out.micro_tempo ));
					// attach the right program to the event
					_allChannelsEvents[pmidi_out.chan]->at(voiceIndex)->at(
						_allChannelsEvents[pmidi_out.chan]->at(voiceIndex)->size()-1 )->_program = programCache[pmidi_out.chan];
					break;
				}
			}
			break;
		case PMIDI_STATUS_DUMMY:
		case PMIDI_STATUS_ROOT:
			std::cout<<" Midi-Format: "<<pmidi_out.format
				<<"  Tracks: "<<pmidi_out.tracks
				<<"  Time Base: "<<pmidi_out.time_base<<std::endl;
			break;
		case PMIDI_STATUS_CONTROL:
			break;
		case PMIDI_STATUS_PROGRAM:
			std::cout<<" Program: "<<pmidi_out.program
				<<"  Channel: "<<pmidi_out.chan
				<<std::endl;
			programCache[pmidi_out.chan] = pmidi_out.program;
			break;
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
//#endif
	writeMidiFileEventsToScore_New( sheet );
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

	// for debugging only:
	for (int i=0;i<_allChannelsTimeSignatures.size();i++) {
		std::cout<<"Time signature "
			<<_allChannelsTimeSignatures[i]->_top
			<<"/"
			<<_allChannelsTimeSignatures[i]->_bottom
			<<" at "
			<<_allChannelsTimeSignatures[i]->_time
			<<std::endl;
	}
	// prepare corresponding lists for time signatures each possible staff, midiimport das a staff for each channel
	for (int chanIndex=0;chanIndex<16;chanIndex++) {
			_allTimeSignatureMusElements << new QList<CAMusElement*>;
	}

	// Calculate the medium pitch for every staff for the key selection later
	_numberOfAllVoices = 2;	// plus one for preprocessing, thats calling pmidi, and one for postprocessing
	for (int chanIndex=0;chanIndex<16;chanIndex++) {
		int n=0;
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
			_numberOfAllVoices += _allChannelsEvents[chanIndex]->size();
		}
	}

	// Quantization on hundredtwentyeighths of time starts and lengths by zeroing the msbits, quant being always a power of two
	const int quant = CAPlayableLength::playableLengthToTimeLength( CAPlayableLength::HundredTwentyEighth /* CAPlayableLength::SixtyFourth */ );
	for (int ch=0;ch<16;ch++) {
		for (voiceIndex=0;voiceIndex<_allChannelsEvents[ch]->size();voiceIndex++) {
			for (int i=0;i< _allChannelsEvents[ch]->at(voiceIndex)->size();i++) {
				_allChannelsEvents[ch]->at(voiceIndex)->at(i)->_time += quant/2;	// rounding
				_allChannelsEvents[ch]->at(voiceIndex)->at(i)->_time &= ~(quant-1);	// quant is power of two
				_allChannelsEvents[ch]->at(voiceIndex)->at(i)->_length += quant/2;
				//_allChannelsEvents[ch]->at(voiceIndex)->at(i)->_length += quant/4;	// preference not to shrink, but make a little longer
				_allChannelsEvents[ch]->at(voiceIndex)->at(i)->_length &= ~(quant-1);
			}
		}
	}

	// Zero _tempo when no tempo change, only in the first voice, so later we will set tempo at the remaining points.
	// By the algorithm used tempo changes on a note will be placed already on the rest before it eventually.
	for (int ch=0;ch<16;ch++) {
		if (_allChannelsEvents[ch]->size() > 0 && _allChannelsEvents[ch]->at(0)->size() > 1) {
			int te = _allChannelsEvents[ch]->at(0)->at(0)->_tempo;
			for (int i=1;i< _allChannelsEvents[ch]->at(0)->size();i++) {
				if( _allChannelsEvents[ch]->at(0)->at(i)->_tempo == te ) {
					_allChannelsEvents[ch]->at(0)->at(i)->_tempo = 0;
				} else {
					te = _allChannelsEvents[ch]->at(0)->at(i)->_tempo;
				}
			}
		}
	}

	// Midi files support support only timesignatures for the all staffs, so, if one staff has a change, all change the same.
	// Result: For time signatures we keep only one list for all staffs.
	// Canorus has separate time signature elements for each staff, but shared vor all voices. Each voice gets a reference copy of the
	// the associated time signature.
	// Add a default time signature if there is none supplied by the imported file.
	//
	// Still missing: A check of time signature consistency
	//
	if ( !_allChannelsTimeSignatures.size() ) {
		_allChannelsTimeSignatures << new CAMidiImportEvent( true, 0, 0, 0, 0, 0, 0 );
		_allChannelsTimeSignatures[_allChannelsTimeSignatures.size()-1]->_top = 4;
		_allChannelsTimeSignatures[_allChannelsTimeSignatures.size()-1]->_bottom = 4;
	}

	int nImportedVoices=1;	// one because preprocessing, ie calling pmidi, is already done
	setProgress(_numberOfAllVoices ? nImportedVoices*100/_numberOfAllVoices : 50 );;

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
		CAMusElement *musElemClef;
		CAMusElement *musElemKeySig;
		CAMusElement *musElemTimeSig;
		for (int voiceIndex=0;voiceIndex<_allChannelsEvents[ch]->size();voiceIndex++) {
			// voiceName = QObject::tr("Voice%1").arg( voiceNumber );
			voice = new CAVoice( "", staff, CANote::StemNeutral, 1 );
			staff->addVoice( voice );
			setCurVoice(voice);
			voice->setMidiChannel( ch );

			if (voiceIndex==0) {
				if (_allChannelsMediumPitch[ch] < 50) {
					musElemClef = new CAClef(CAClef::Bass, staff, 0, 0 );
				} else if (_allChannelsMediumPitch[ch] < 65) {
					musElemClef = new CAClef(CAClef::Treble, staff, 0, -8 );
				} else {
					musElemClef = new CAClef(CAClef::Treble, staff, 0, 0 );
				}
				CADiatonicKey dk = CADiatonicKey(
					pmidi_out.key, pmidi_out.minor ? CADiatonicKey::Minor : CADiatonicKey::Major );
				musElemKeySig = new CAKeySignature( dk, staff, 0 );

				musElemTimeSig = new CATimeSignature( pmidi_out.top, pmidi_out.bottom, staff, 0 );
			}
			voice->append( musElemClef, false );
			voice->append( musElemKeySig, false );
			//voice->append( musElemTimeSig, false );

			writeMidiChannelEventsToVoice_New( ch, voiceIndex, staff, voice );
			setProgress(_numberOfAllVoices ? nImportedVoices*100/_numberOfAllVoices : 50 );;
			++nImportedVoices;
			staff->synchronizeVoices();
		}

		staffIndex++;
	}
}


/*!
	Docu neeeded, definitively! rud
*/

CAMusElement* CAMidiImport::getOrCreateTimeSignature( int time, int channel, int voiceIndex, CAStaff *staff, CAVoice *voice ) {

	int maxFill = -1;
	int indexMaxFill;

	if (!_allTimeSignatureMusElements[channel]->size()) {
		_actualTimeSignatureIndex = 0;
		int top = _allChannelsTimeSignatures[_actualTimeSignatureIndex]->_top;
		int bottom = _allChannelsTimeSignatures[_actualTimeSignatureIndex]->_bottom;
		_allTimeSignatureMusElements[channel]->append( new CATimeSignature( top, bottom, staff, 0 ));
		std::cout<<"                             neue Timesig at "<<time<<" in Channel "<<channel<<", es gibt "
																<<_allChannelsTimeSignatures.size()
																<<std::endl;
		//voice->append( _allTimeSignatureMusElements[channel]->at( _actualTimeSignatureIndex ));
		return _allTimeSignatureMusElements[channel]->at(_actualTimeSignatureIndex);
	}
	// check if more than one time signature at all
	if (_actualTimeSignatureIndex < 0 || _allChannelsTimeSignatures.size() > _actualTimeSignatureIndex+1) {
		// is a new time signature already looming there?
		if (time >= _allChannelsTimeSignatures[_actualTimeSignatureIndex+1]->_time) {
			_actualTimeSignatureIndex++;	// for each voice we run down the list of time signatures of the sheet, all staffs.
			if (_allTimeSignatureMusElements[channel]->size() >= _actualTimeSignatureIndex+1) {
				//voice->append( _allTimeSignatureMusElements[channel]->at( _actualTimeSignatureIndex ));
				return _allTimeSignatureMusElements[channel]->at(_actualTimeSignatureIndex);
			} else { 
				int top = _allChannelsTimeSignatures[_actualTimeSignatureIndex]->_top;
				int bottom = _allChannelsTimeSignatures[_actualTimeSignatureIndex]->_bottom;
				_allTimeSignatureMusElements[channel]->append( new CATimeSignature( top, bottom, staff, 0 ));
				std::cout<<"                             neue Timesig at "<<time<<" in Channel "<<channel<<", es gibt "
																<<_allChannelsTimeSignatures.size()
																<<std::endl;
				//voice->append( _allTimeSignatureMusElements[channel]->at( _actualTimeSignatureIndex ));
				return _allTimeSignatureMusElements[channel]->at(_actualTimeSignatureIndex);
			}
		}
	}
	return 0;
}

/*!
	Docu neeeded

	Apropo program support at midi import: Now the last effective program assignement per voice will make it through.
	A separation in voices regarding the midi program is note yet implemented.
*/

void CAMidiImport::writeMidiChannelEventsToVoice_New( int channel, int voiceIndex, CAStaff *staff, CAVoice *voice ) {

	QList<CAMidiImportEvent*> *events = _allChannelsEvents[channel]->at(voiceIndex);
	CANote *note;
	CARest *rest;
	CANote *previousNote;	// for sluring
	QList<CAPlayableLength> lenList;	// work list when splitting notes and rests at barlines
	CATimeSignature *ts = 0;
	CABarline *b = 0;
	int time = 0;			// current time in the loop, only increasing, for tracking notes and rests
	int length;
	int pitch;
	int program;
	int tempo = 0;

	_actualTimeSignatureIndex = -1;	// for each voice we run down the list of time signatures of the sheet, all staffs.

	std::cout<< "Channel "<<channel<<" VoiceIndex "<<voiceIndex<<"  "<<std::setw(5)<<events->size()<<" elements"<<std::endl;

	for (int i=0; i<events->size(); i++ ) {

		// we place a tempo mark only for the first voice, and if we don't place we set tempo null
		int tempo = voiceIndex == 0 ? events->at(i)->_tempo : 0;
		
		b = static_cast<CABarline*>( voice->previousByType( CAMusElement::Barline,
			voice->lastMusElement()));


		CAMusElement *tsElem = getOrCreateTimeSignature( time, channel, voiceIndex, staff, voice );
		if (tsElem) {
			voice->append( tsElem, false );
			ts = static_cast<CATimeSignature*>(tsElem);
			if (channel==9 && voiceIndex > 1) {
				//std::cout<< "    in advance new time sig at "<<time<<" in "<<ts->beat()<<"/"<<ts->beats()<<std::endl;
			}
		}

		// check if we need to add rests
		length = events->at(i)->_time - time;
		while (length > 0) {

			// This definitively needs clean up!!!
			CAMusElement *fB = voice->getOnePreviousByType( CAMusElement::Barline, time );
			if (fB) {
				b = static_cast<CABarline*>(fB);
			} else {
				b = 0;
			}
			// Hier wird eine vergangene Taktlinie zugewiesen:  b = static_cast<CABarline*>( voice->previousByType( CAMusElement::Barline, voice->lastMusElement()));
			b = static_cast<CABarline*>( voice->previousByType( CAMusElement::Barline, voice->lastMusElement()));

			lenList.clear();
			lenList << CAPlayableLength::matchToBars( length, voice->lastTimeEnd(), b, ts );

			for (int j=0; j<lenList.size(); j++) {
				rest = new CARest( CARest::Normal, lenList[j], voice, 0, -1 );
				voice->append( rest, false );
				int len = CAPlayableLength::playableLengthToTimeLength( lenList[j] );
				//std::cout<< "    Rest Length "<<len<<" at "<<time<<std::endl;
				time += len;
				length -= len;
				if ( tempo ) {
					CAMark *_curMark = new CATempo( CAPlayableLength::Quarter, tempo, rest);
					rest->addMark(_curMark);
					tempo = 0;
				}
				tsElem = getOrCreateTimeSignature( time, channel, voiceIndex, staff, voice );
				if (tsElem) {
					voice->append( tsElem, false );
					ts = static_cast<CATimeSignature*>(tsElem);
					std::cout<< "    for a rest new time sig at "<<time<<" in "<<ts->beat()<<"/"<<ts->beats()<<std::endl;
				}
				// Barlines are shared among voices, see we append an existing one, otherwise a new one
				//QList<CAMusElement*> foundBarlines = staff->getEltByType( CAMusElement::Barline, rest->timeEnd() );
				CAMusElement *bl = staff->getOneEltByType( CAMusElement::Barline, rest->timeEnd() );
				if (bl) {
					voice->append( bl, false );
					b = static_cast<CABarline*>(bl);
				} else {
			  		musElementFactory()->placeAutoBar( rest );
				}
				if (tsElem) {
					;
					break;
				}
			}
		}
		// notes to be added
		length = events->at(i)->_length;
		pitch = events->at(i)->_pitch;
		program = events->at(i)->_program;
		previousNote = 0;
		while ( length > 0 && pitch > 0 && events->at(i)->_velocity > 0 ) {

			// this needs clean up, definitevely
			CAMusElement *fB = voice->getOnePreviousByType( CAMusElement::Barline, time );
			if (fB) {
				b = static_cast<CABarline*>(fB);
			} else {
				b = 0;
			}
			b = static_cast<CABarline*>( voice->previousByType( CAMusElement::Barline, voice->lastMusElement()));
			
			lenList.clear();
			lenList << CAPlayableLength::matchToBars( length, voice->lastTimeEnd(), b, ts );

			for (int j=0; j<lenList.size();j++) {
				CADiatonicPitch diaPitch = matchPitchToKey( voice, CAMidiDevice::midiPitchToDiatonicPitch(pitch) );
				note = new CANote( diaPitch, lenList[j], voice, -1 );
				voice->append( note, false );
				voice->setMidiProgram( program );
				int len = CAPlayableLength::playableLengthToTimeLength( lenList[j] );
				//std::cout<< "    Note Length "<<len<<" at "<<time<<std::endl;
				time += len;
				length -= len;
				if ( tempo ) {
					CAMark *_curMark = new CATempo( CAPlayableLength::Quarter, tempo, note);
					note->addMark(_curMark);
					tempo = 0;
				}
				tsElem = getOrCreateTimeSignature( note->timeEnd(), channel, voiceIndex, staff, voice );
				if (tsElem) {
					voice->append( tsElem, false );
					ts = static_cast<CATimeSignature*>(tsElem);
					//std::cout<< "    for a note new time sig at "<<time<<" in "<<ts->beat()<<"/"<<ts->beats()<<std::endl;
				}
				// Barlines are shared among voices, see we append an existing one, otherwise a new one
				CAMusElement * bl = staff->getOneEltByType( CAMusElement::Barline, note->timeEnd() );
				if (bl) {
					voice->append( bl, false );
					b = static_cast<CABarline*>(bl);
				} else {
			  		musElementFactory()->placeAutoBar( note );
				}
				if (previousNote) {
					CASlur *slur = new CASlur( CASlur::TieType, CASlur::SlurPreferred, staff, previousNote, note );
					previousNote->setTieStart( slur );
					note->setTieEnd( slur );
				}
				previousNote = note;
				if (tsElem) {
					;
					break;
				}
			}
		}
	}
}


void CAMidiImport::closeFile() {
	file()->close();
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



