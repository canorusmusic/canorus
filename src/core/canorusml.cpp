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
#include "core/rest.h"
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
	
	//fix voice errors
	for (int i=0; i<doc->sheetCount(); i++) {
		for (int j=0; j<doc->sheetAt(i)->staffCount(); j++) {
			doc->sheetAt(i)->staffAt(j)->fixVoiceErrors();
		}
	}
	
	delete canHandler;
}

const QString CACanorusML::createMLVoice(CAVoice *v) {
	QString voiceString;
	int lastNotePitch = 28;
	int firstNotePitchInChord;
	int curStreamTime = 0;
	QString lastPlayableLength;
	int lastPlayableDotted=0;
	
	for (int i=0; i<v->musElementCount(); i++, voiceString += " ") {
		//(CAMusElement)
		switch (v->musElementAt(i)->musElementType()) {
			case CAMusElement::Clef: {
				//CAClef
				CAClef *clef = (CAClef*)v->musElementAt(i);
				if (clef->timeStart()!=curStreamTime) break;	//TODO: If the time isn't the same, insert hidden rests to fill the needed time
				voiceString += "<clef>";
				voiceString += clef->clefTypeML();
				voiceString += "</clef>";
				
				lastNotePitch = clef->centerPitch();
				break;
			}
			case CAMusElement::KeySignature: {
				//CAKeySignature
				CAKeySignature *key = (CAKeySignature*)v->musElementAt(i);
				if (key->timeStart()!=curStreamTime) break;	//TODO: If the time isn't the same, insert hidden rests to fill the needed time
				voiceString += QString("<key type=\"") + key->diatonicGenderML() + "\">";
				voiceString += key->pitchML();
				voiceString += "</key>";
			
				break;
			}
			case CAMusElement::TimeSignature: {
				//CATimeSignature
				CATimeSignature *time = (CATimeSignature*)v->musElementAt(i);
				if (time->timeStart()!=curStreamTime) break;	//TODO: If the time isn't the same, insert hidden rests to fill the needed time
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
				if (bar->timeStart()!=curStreamTime) break;	//TODO: If the time isn't the same, insert hidden rests to fill the needed time
				if (bar->barlineType() == CABarline::Single)
					voiceString += "|";
				
				break;
			}
			case CAMusElement::Note: {
				//CANote
				CANote *note = (CANote*)v->musElementAt(i);
				if (note->timeStart()!=curStreamTime) break;	//TODO: If the time isn't the same, insert hidden rests to fill the needed time
				if (note->isPartOfTheChord() && note->isFirstInTheChord()) {
					voiceString += "<chord>";
					firstNotePitchInChord=note->pitch();
				}
				
				voiceString += note->pitchML();
				
				int delta = lastNotePitch - note->pitch();
				while (delta > 3) { //add the needed amount of the commas
					voiceString += ",";
					delta -= 7;
				}
				while (delta < -3) { //add the needed amount of the apostrophes
					voiceString += "'";
					delta += 7;
				}
				
				if (lastPlayableLength != note->lengthML() || lastPlayableDotted != note->dotted()) {
					voiceString += note->lengthML();
					for (int j=0; j<note->dotted(); j++)
						voiceString += ".";
				}
				

				lastNotePitch = note->pitch();
				lastPlayableLength = note->lengthML();
				lastPlayableDotted = note->dotted();
				
				//finish the chord stanza if that's the last note of the chord
				if (note->isPartOfTheChord() && note->isLastInTheChord()) {
					voiceString += "</chord>";
					lastNotePitch = firstNotePitchInChord;
				}
				
				//add to the stream time, if the note is not part of the chord or is the last one in the chord
				if (!note->isPartOfTheChord() ||
				    (note->isPartOfTheChord() && note->isLastInTheChord()) )
					curStreamTime += note->timeLength();
				
				break;
			}
			case CAMusElement::Rest: {
				//CARest
				CARest *rest = (CARest*)v->musElementAt(i);
				if (rest->timeStart()!=curStreamTime) break;	//TODO: If the time isn't the same, insert hidden rests to fill the needed time
				
				voiceString += rest->restTypeML();
				
				if (lastPlayableLength!=rest->lengthML() || lastPlayableDotted!=rest->dotted()) {
					voiceString += rest->lengthML();
					for (int j=0; j<rest->dotted(); j++)
						voiceString += ".";
				}
				
				lastPlayableLength = rest->lengthML();
				curStreamTime += rest->timeLength();
				lastPlayableDotted = rest->dotted();
				
				break;
			}
		}
	}
	
	voiceString.truncate(voiceString.length()-1);	//remove the trailing blank at the end of the voice
	return voiceString;
}

bool CACanorusML::fatalError (const QXmlParseException & exception) {
	qWarning() << "Fatal error on line " << exception.lineNumber()
		<< ", column " << exception.columnNumber() << ": "
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
	} else if (qName == "chord") {}
	else if (qName == "clef") {}
	else if (qName == "time") {
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
	
	//Every voice *must* contain signs on their own (eg. a clef is placed in all voices, not just the first one).
	//The following code finds a sign with the same properties at the same time in other voices. If such a sign exists, only place a pointer to this sign in the current voice. Otherwise, add a sign to all the voices read so far.
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
		
		//lookup an element with the same type at the same time
		QList<CAMusElement*> foundElts = ((CAStaff*)_curContext)->getEltByType(CAMusElement::Clef, _curVoice->lastTimeEnd());
		CAMusElement *sign=0;
		CAClef *clef = new CAClef(_cha, _curVoice->staff(), _curVoice->lastTimeEnd());
		for (int i=0; i<foundElts.size(); i++) {
			if (!foundElts[i]->compare(clef))	//element has exactly the same properties
				if (!_curVoice->contains(foundElts[i]))	{ //element isn't present in the voice yet
					sign = foundElts[i];
					break;
				}
		}
			
		if (!sign) {
			//the element doesn't exist yet - add it
			_curVoice->staff()->insertSignAfter(clef, _curVoice->musElementCount()?_curVoice->lastMusElement():0, true);
		} else {
			//the element was found, insert only a reference to the current voice
			_curVoice->appendMusElement(sign);
			delete clef;
		}
		_cha="";
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
		
		//lookup an element with the same type at the same time
		QList<CAMusElement*> foundElts = ((CAStaff*)_curContext)->getEltByType(CAMusElement::KeySignature, _curVoice->lastTimeEnd());
		CAMusElement *sign=0;
		CAKeySignature *key = new CAKeySignature(_cha, _diatonicGender, _curVoice->staff(), _curVoice->lastTimeEnd());
		for (int i=0; i<foundElts.size(); i++) {
			if (!foundElts[i]->compare(key))	//element has exactly the same properties
				if (!_curVoice->contains(foundElts[i]))	{ //element isn't present in the voice yet
					sign = foundElts[i];
					break;
				}
		}
			
		if (!sign) {
			//the element doesn't exist yet - add it
			_curVoice->staff()->insertSignAfter(key, _curVoice->musElementCount()?_curVoice->lastMusElement():0, true);
		} else {
			//the element was found, insert only a reference to the current voice
			_curVoice->appendMusElement(sign);
			delete key;
		}
		_diatonicGender="";
		_cha="";
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
		
		//lookup an element with the same type at the same time
		QList<CAMusElement*> foundElts = ((CAStaff*)_curContext)->getEltByType(CAMusElement::TimeSignature, _curVoice->lastTimeEnd());
		CAMusElement *sign=0;
		CATimeSignature *time = new CATimeSignature(_cha, _curVoice->staff(), _curVoice->lastTimeEnd(), _timeSignatureType);
		for (int i=0; i<foundElts.size(); i++) {
			if (!foundElts[i]->compare(time))	//element has exactly the same properties
				if (!_curVoice->contains(foundElts[i]))	{ //element isn't present in the voice yet
					sign = foundElts[i];
					break;
				}
		}
			
		if (!sign) {
			//the element doesn't exist yet - add it
			_curVoice->staff()->insertSignAfter(time, _curVoice->musElementCount()?_curVoice->lastMusElement():0, true);
		} else {
			//the element was found, insert only a reference to the current voice
			_curVoice->appendMusElement(sign);
			delete time;
		}
		_cha="";
	}
	
	_depth.pop();
	return true;
}

bool CACanorusML::characters(const QString& ch) {
	_cha = ch;
	if (_depth.top()=="voice" || _depth.top()=="chord")
		return readMusElements(_cha);
	
	return true;
}

bool CACanorusML::readMusElements(QString string) {
	string = string.simplified().toUpper();	//remove weird whitespaces throughout the string and replace them by a single blank
	for (int eltIdx=0; string.size(); eltIdx++) {
		int idx2 = string.indexOf(" ");	//find the index of the next music element
		if (idx2==-1)
			idx2 = string.size();
		
		if (QString(string[0]).contains(QRegExp("[A-G]"))) {	//if the first char is [A-G] letter, it's always the note pitch
			//CANote
			int curPitch;
			int curLength;
			int curDotted=0;
			signed char curAccs = 0;

			//determine pitch
			bool inChord = ((_depth.last()!="chord") || (eltIdx==0));
			curPitch = string[0].toLatin1() - 'A' + 5	//determine the 0-6 pitch from note name
				- (((_curVoice->lastNotePitch(inChord)==-1)?28:_curVoice->lastNotePitch(inChord)) % 7);	//get the delta of the last note pitch. If there is not last note, take the 28 as default height (C1)
			while (curPitch<-3)	//normalize pitch - the max +/- interval is fourth
				curPitch+=7;
			while (curPitch>3)
				curPitch-=7;
			curPitch += (_curVoice->lastNotePitch(inChord)==-1)?28:_curVoice->lastNotePitch(inChord);
		
			//determine accidentals
			while ((string.indexOf("IS") != -1) && (string.indexOf("IS") < idx2)) {
				curAccs++;
				int delta;
				string = string.remove(0, delta = string.indexOf("IS") + 2);
				idx2 -= delta;
			}
			while ((string.indexOf("ES") != -1) && (string.indexOf("ES") < idx2)) {
				curAccs--;
				int delta;
				string = string.remove(0, delta = string.indexOf("ES") + 2);
				idx2 -= delta;
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
			if ((lIdx == -1) || (lIdx > idx2)) {	//no length written
				curLength = _curVoice->lastPlayableElt()->playableLength();
				curDotted = _curVoice->lastPlayableElt()->dotted();
				if (curLength==-1)
					curLength=CAPlayable::Quarter;
			} else {								//length written
				int d;
				for (d = string.indexOf(".",lIdx); d!=-1 && d<idx2 && string[d]=='.'; d++)
					curDotted++;
				if (d<=0 || d>idx2)	//no dots were found
					d=idx2;
				else
					d--;
				curLength = string.mid(lIdx,d-lIdx).toInt();
			}
			
			CANote *note;
			if (_depth.last()!="chord" || eltIdx==0) //the note is not part of the chord or is the first note in the chord
				note = new CANote((CAPlayable::CAPlayableLength)curLength, _curVoice, curPitch, curAccs, _curVoice->lastTimeEnd(), curDotted);
			else	//the note is part of the already built chord
				note = new CANote((CAPlayable::CAPlayableLength)curLength, _curVoice, curPitch, curAccs, _curVoice->lastTimeStart(), curDotted);
			
			_curVoice->appendMusElement(note);
		} else if (string[0]=='R' || string[0]=='S') {
			//CARest
			int lIdx = string.indexOf(QRegExp("[0-9]"));
			CAPlayable::CAPlayableLength curLength;
			int curDotted = 0;
			
			if ((lIdx == -1) || (lIdx > idx2)) {
				curLength = _curVoice->lastPlayableElt()->playableLength();
				curDotted = _curVoice->lastPlayableElt()->dotted();
				if (curLength==-1)
					curLength=CAPlayable::Quarter;
			} else {
				int d;
				for (d = string.indexOf(".",lIdx); d!=-1 && d<idx2 && string[d]=='.'; d++)
					curDotted++;
				if (d<=0 || d>idx2)	//no dots were found
					d=idx2;
				else
					d--;
				curLength = (CAPlayable::CAPlayableLength)string.mid(lIdx,d-lIdx).toInt();
			}

			_curVoice->appendMusElement(new CARest((string[0]=='R'?CARest::Normal:CARest::Hidden), curLength, _curVoice, _curVoice->lastTimeEnd(), curDotted));
		} else if (string[0]=='|') {
			//CABarline
			//lookup an element with the same type at the same time
			QList<CAMusElement*> foundElts = _curVoice->staff()->getEltByType(CAMusElement::Barline, _curVoice->lastTimeEnd());
			CAMusElement *sign=0;
			CABarline *bar = new CABarline(CABarline::Single, _curVoice->staff(), _curVoice->lastTimeEnd());
			for (int i=0; i<foundElts.size(); i++) {
				if (!foundElts[i]->compare(bar))	//element has exactly the same properties
					if (!_curVoice->contains(foundElts[i]))	{ //element isn't present in the voice yet
						sign = foundElts[i];
						break;
					}
			}
			
			if (!sign) {
				//the element doesn't exist yet - add it
				_curVoice->staff()->insertSignAfter(bar, _curVoice->musElementCount()?_curVoice->lastMusElement():0, true);
			} else {
				//the element was found, insert only a reference to the current voice
				_curVoice->appendMusElement(sign);
				delete bar;
			}
		}
		
		string = string.remove(0, idx2+1);
	}
	
	return true;
}
