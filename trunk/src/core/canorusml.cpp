/** @file canorusml.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Georg Rudolph, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QtXml>

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

CACanorusML::CACanorusML(CADocument *doc, CAMainWin *mainWin) {
	_document = doc;
	_mainWin = mainWin;
	
	_curSheet = 0;
	_curContext = 0;
	_curVoice = 0;
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
					out << idn(depth++) << "<staff instrumentName=\"" << staff->instrumentName() << "\">\n";
					
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

void CACanorusML::openDocument(QIODevice* in, CADocument *doc, CAMainWin *mainWin) {
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
	
	for (int i=0; i<v->musElementCount(); i++) {
		//(CAMusElement)
		switch (v->musElementAt(i)->musElementType()) {
			case CAMusElement::Clef: {
				//CAClef
				CAClef *clef = (CAClef*)v->musElementAt(i);
				voiceString += "<clef>";
				voiceString += clef->clefTypeML();
				voiceString += "</clef> ";
			
				break;
			}
			case CAMusElement::KeySignature: {
				//CAKeySignature
				CAKeySignature *key = (CAKeySignature*)v->musElementAt(i);
				voiceString += QString("<key type=\"") + key->diatonicGenderML() + "\">";
				voiceString += key->pitchML();
				voiceString += "</key> ";
			
				break;
			}
			case CAMusElement::Note: {
				//CANote
				CANote *note = (CANote*)v->musElementAt(i);
				voiceString += note->pitchML();
				
				int delta = lastPitch - note->pitch();
				if (delta > 3) {	//add the needed amount of the commas
					while (delta > 0) {
						voiceString += ",";
						delta -= 7;
					}
				} else if (delta < -3) {	//add the needed amount of the apostrophes
					while (delta < 0) {
						voiceString += "'";
						delta += 7;
					}
				}
				
				if (lastLength != note->lengthML())
					voiceString += note->lengthML();
				
				voiceString += " ";
				lastPitch = note->pitch();
				lastLength = note->lengthML();
			}
		}
	}
	
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
	} else if (qName == "key") {
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
	}
	
	_depth.pop();
	return true;
}

bool CACanorusML::characters(const QString& ch) {
	_cha = ch;
	if (_depth.top()=="voice")
		readMusElements(_cha);
	
	return true;
}

void CACanorusML::readMusElements(const QString string) {
}
