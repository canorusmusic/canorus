/** @file canorusml.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Georg Rudolph, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QXmlInputSource>

#include <iostream>	//DEBUG

#include "ui/mainwin.h"

#include "core/canorusml.h"
#include "core/sheet.h"
#include "core/context.h"
#include "core/staff.h"
#include "core/voice.h"
#include "core/note.h"
#include "core/clef.h"
#include "core/muselement.h"
#include "core/keysignature.h"
#include "core/timesignature.h"
#include "core/barline.h"

CACanorusML::CACanorusML(CADocument *doc, CAMainWin *mainWin) {
	_document = doc;
	_mainWin = mainWin;
	
	_curSheet = 0;
	_curContext = 0;
	_curVoice = 0;
	
	_diatonicGender = -1;
}

CACanorusML::~CACanorusML() {
}

void CACanorusML::saveDocument(QTextStream& out, CADocument *doc) {
	int depth = 0;

	out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	
	//CADocument start
	out << idn(depth++) << "<document";
	if (!doc->title().isEmpty())
		out << " title=\"" << doc->title() << "\"";
	if (!doc->composer().isEmpty())
		out << " composer=\"" << doc->composer() << "\"";
	out << ">\n";
		
	for (int sheetIdx=0; sheetIdx < doc->sheetCount(); sheetIdx++) {
		//CASheet start
		out << idn(depth++) << "<sheet";
		out << " name=\"" << doc->sheetAt(sheetIdx)->name() << "\"";
		out << ">\n";
		
		for (int contextIdx=0; contextIdx < doc->sheetAt(sheetIdx)->contextCount(); contextIdx++) {
			//(CAContext)
			CAContext *c = doc->sheetAt(sheetIdx)->contextAt(contextIdx);
			
			switch (c->contextType()) {
				case CAContext::Staff:
					//CAStaff
					CAStaff *staff = (CAStaff*)c;
					out << idn(depth++) << "<staff name=\"" << staff->name() << "\">\n";
					
					for (int voiceIdx=0; voiceIdx < staff->voiceCount(); voiceIdx++) {
						//CAVoice
						CAVoice *v = staff->voiceAt(voiceIdx);
						out << idn(depth++) << "<voice name=\"" << v->name() << "\" midiChannel=\"" << v->midiChannel() << "\" midiProgram=\"" << v->midiProgram() << "\">\n";

						out << idn(depth) << createMLVoice(v) << "\n";	//write down the actual contents of the voice
						
						//CAVoice end
						out << idn(--depth) << "</voice>\n";
					}
					
					//CAStaff end
					out << idn(--depth) << "</staff>\n";
					break;
			}
		}
		
		//CASheet end
		out << idn(--depth) << "</sheet>\n";
	}
	
	//CADocument end
	out << idn(--depth) << "</document>\n";
}

void CACanorusML::openDocument(QXmlInputSource* in, CADocument *doc, CAMainWin *mainWin) {
	QXmlSimpleReader reader;
	CACanorusML *canHandler = new CACanorusML(doc, mainWin);
	reader.setContentHandler(canHandler);
	reader.parse(in);
	
	delete canHandler;
}

const QString CACanorusML::createMLVoice(CAVoice *v) {
	QString voiceString;
	int lastPitch = 28;
	QString lastLength = "";
	
	for (int i=0; i<v->musElementCount(); i++, voiceString += " ") {
		//(CAMusElement)
		switch (v->musElementAt(i)->musElementType()) {
			case CAMusElement::Clef: {
				//CAClef
				CAClef *clef = (CAClef*)v->musElementAt(i);
				voiceString += "<clef>";
				voiceString += clef->clefTypeML();
				voiceString += "</clef>";
				
				lastPitch = clef->centerPitch();
				break;
			}
			case CAMusElement::KeySignature: {
				//CAKeySignature
				CAKeySignature *key = (CAKeySignature*)v->musElementAt(i);
				voiceString += QString("<key type=\"") + key->diatonicGenderML() + "\">";
				voiceString += key->pitchML();
				voiceString += "</key>";
			
				break;
			}
			case CAMusElement::TimeSignature: {
				//CATimeSignature
				CATimeSignature *time = (CATimeSignature*)v->musElementAt(i);
				voiceString += QString("<time");
				if (time->timeSignatureType() != CATimeSignature::Classical)
					voiceString += QString(" type=\"") + time->timeSignatureTypeML() + QString("\"");
				voiceString += QString(">");
				voiceString += time->timeSignatureML();
				voiceString += "</time>";
			
				break;
			}
			case CAMusElement::Barline: {
				//CABarline
				CABarline *bar = (CABarline*)v->musElementAt(i);
				if (bar->barlineType() == CABarline::Single)
					voiceString += "|";
				
				break;
			}
			case CAMusElement::Note: {
				//CANote
				CANote *note = (CANote*)v->musElementAt(i);
				voiceString += note->pitchML();
				
				int delta = lastPitch - note->pitch();
				while (delta > 3) { //add the needed amount of the commas
					voiceString += ",";
					delta -= 7;
				}
				while (delta < -3) { //add the needed amount of the apostrophes
					voiceString += "'";
					delta += 7;
				}
				
				if (lastLength != note->lengthML())
					voiceString += note->lengthML();
				
				lastPitch = note->pitch();
				lastLength = note->lengthML();
			}
		}
	}
	
	voiceString.truncate(voiceString.length()-1);	//remove the trailing blank
	return voiceString;
}

bool CACanorusML::fatalError (const QXmlParseException & exception) {
	qWarning() << "Fatal error on line" << exception.lineNumber()
		<< ", column" << exception.columnNumber() << ":"
		<< exception.message() << "\n\nParser message:\n" << _errorMsg;
	
	return false;
}

bool CACanorusML::startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& attributes) {
	if (!_document) {
		_errorMsg = "The document doesn't exist yet!";
		return false;
	}

	if (qName == "sheet") {
		//CASheet
		QString sheetName = attributes.value("name");
		
		if (!(_curSheet = _document->sheet(sheetName))) {	//if the document doesn't contain the sheet with the given name, create a new sheet and add it to the document. Otherwise, just set the current sheet to the found one and leave
			if (sheetName.isEmpty())
				sheetName = QString("Sheet") + " " + QString::number(_document->sheetCount()+1);
			CASheet *sheet = new CASheet(sheetName, _document);
			_curSheet = sheet;
			_document->addSheet(sheet);
			_mainWin->addSheet(sheet);
		}
	} else if (qName == "staff") {
		//CAStaff
		QString staffName = attributes.value("name");
		if (!_curSheet) {
			_errorMsg = "The sheet where to add the staff doesn't exist yet!";
			return false;
		}
		
		if (!(_curContext = _curSheet->context(staffName))) {	//if the sheet doesn't contain the staff with the given name, create a new sheet and add it to the document. Otherwise, just set the current staff to the found one and leave
			if (staffName.isEmpty())
				staffName = QString("Staff") + " " + QString::number(_curSheet->staffCount()+1);
			CAStaff *staff = new CAStaff(_curSheet, staffName);
			_curContext = staff;
			_curSheet->addContext(staff);
		}
	} else if (qName == "voice") {
		//CAVoice
		QString voiceName = attributes.value("name");
		if (!_curContext) {
			_errorMsg = "The context where the voice should be added doesn't exist yet!";
			return false;
		} else if (_curContext->contextType() != CAContext::Staff) {
			_errorMsg = "The context type which will contain voice isn't staff!";
			return false;
		}
		
		if (!(_curVoice = ((CAStaff*)_curContext)->voice(voiceName))) {	//if the staff doesn't contain the voice with the given name, create a new voice and add it to the document. Otherwise, just set the current voice to the found one and leave
			if (voiceName.isEmpty())
				voiceName = QString("Voice") + " " + QString::number(((CAStaff*)_curContext)->voiceCount()+1);
			CAVoice *voice = new CAVoice((CAStaff*)_curContext, voiceName);
			_curVoice = voice;
			((CAStaff*)_curContext)->addVoice(voice);
		}
	} else if (qName == "clef") {
	} else if (qName == "time") {
		//CATimeSignature
		_timeSignatureType = attributes.value("type");
	} else if (qName == "key") {
		//CAKeySignature
		_diatonicGender = attributes.value("type");
	}
	
	_depth.push(qName);
	return true;
}

bool CACanorusML::endElement(const QString& namespaceURI, const QString& localName, const QString& qName) {
	if (!_document) {
		_errorMsg = "The document doesn't exist yet!";
		return false;
	}

	if (qName == "clef") {
		//CAClef
		if (!_curContext) {
			_errorMsg = "No staffs exist yet to place a clef!";
			return false;
		} else if (_curContext->contextType()!=CAContext::Staff) {
			_errorMsg = "The context where the clef should be added isn't a staff!";
			return false;
		} else if (((CAStaff*)_curContext)->voiceCount()==0) {
			_errorMsg = "At least one voice should exist in order to add a clef!";
			return false;
		}

		CAClef *clef = new CAClef(_cha, _curVoice->staff(), _curVoice->lastTimeEnd());
		_curVoice->staff()->insertSign(clef);
	} else if (qName == "key") {
		//CAKeySignature
		if (!_curContext) {
			_errorMsg = "No staffs exist yet to place a key!";
			return false;
		} else if (_curContext->contextType()!=CAContext::Staff) {
			_errorMsg = "The context where the key signature should be added isn't a staff!";
			return false;
		} else if (((CAStaff*)_curContext)->voiceCount()==0) {
			_errorMsg = "At least one voice should exist in order to add a key signature!";
			return false;
		}
		
		CAKeySignature *keySig = new CAKeySignature(_cha, _diatonicGender, _curVoice->staff(), _curVoice->lastTimeEnd());
		_diatonicGender="";
		_cha="";
		_curVoice->staff()->insertSign(keySig);
	} else if (qName == "time") {
		//CATimeSignature
		if (!_curContext) {
			_errorMsg = "No staffs exist yet to place a time signature!";
			return false;
		} else if (_curContext->contextType()!=CAContext::Staff) {
			_errorMsg = "The context where the time signature should be added isn't a staff!";
			return false;
		} else if (((CAStaff*)_curContext)->voiceCount()==0) {
			_errorMsg = "At least one voice should exist in order to add a time signature!";
			return false;
		}

		CATimeSignature *time = new CATimeSignature(_cha, _curVoice->staff(), _curVoice->lastTimeEnd(), _timeSignatureType);
		
		_curVoice->staff()->insertSign(time);
	}
	
	_depth.pop();
	return true;
}

bool CACanorusML::characters(const QString& ch) {
	_cha = ch;
	if (_depth.top()=="voice")
		return readMusElements(_cha);
	
	return true;
}

bool CACanorusML::readMusElements(QString string) {
	string = string.simplified().toUpper();	//remove weirs whitespaces throughout the string and replace them by a single blank
	while (string.size()) {
		int idx2 = string.indexOf(" ");	//find the index of the next note
		if (idx2==-1)
			idx2 = string.size();

		//CANote
		if (QString(string[0]).contains(QRegExp("[A-G]"))) {	//if the first char is [A-G] letter, it's always the note pitch
			int curPitch;
			int curLength;
			signed char curAccs = 0;

			//determine pitch
			curPitch = string[0].toLatin1() - 'A' + 5	//determine the 0-6 pitch from note name
				- (((_curVoice->lastNotePitch()==-1)?28:_curVoice->lastNotePitch()) % 7);	//get the delta of the last note pitch. If there is not last note, take the 28 as default height (C1)
			while (curPitch<-3)	//normalize pitch - the max +/- interval is fourth
				curPitch+=7;
			while (curPitch>3)
				curPitch-=7;
			curPitch += (_curVoice->lastNotePitch()==-1)?28:_curVoice->lastNotePitch();
		
			//determine accidentals
			while ((string.indexOf("IS") != -1) && (string.indexOf("IS") < idx2)) {
				curAccs++;
				string = string.remove(0, string.indexOf("IS") + 2);
				idx2 -= 2;
			}
			while ((string.indexOf("ES") != -1) && (string.indexOf("ES") < idx2)) {
				curAccs--;
				string = string.remove(0, string.indexOf("ES") + 2);
				idx2 -= 2;
			}
			
			//add octave up/down
			for (int i=0; i<idx2; i++) {
				if (string[i]=='\'') {
					curPitch+=7;
				} else if (string[i]==',') {
					curPitch-=7;
				}
			}
			
			//determine note length
			int lIdx = string.indexOf(QRegExp("[0-9]"));
			if ((lIdx == -1) || (lIdx > idx2)) {
				curLength = _curVoice->lastNoteLength();
				if (curLength==-1)
					curLength=CANote::Quarter;
			} else {
				curLength = string.mid(lIdx,idx2-lIdx).toInt();
			}
			
			CANote *note = new CANote((CANote::CANoteLength)curLength, _curVoice, curPitch, curAccs, _curVoice->lastTimeEnd());
			_curVoice->insertMusElement(note);
		} else
		//CABarline
		if (string[0]=='|') {
			_curVoice->insertMusElement(new CABarline(CABarline::Single, (CAStaff*)_curContext, _curVoice->lastTimeEnd()));
		}
		
		string = string.remove(0, idx2+1);
	}
	
	return true;
}
