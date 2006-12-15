/** @file canorusml.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Georg Rudolph, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QXmlInputSource>
#include <QDomDocument>

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

CACanorusML::CACanorusML(CAMainWin *mainWin) {
	_mainWin = mainWin;
	
	_document = 0;
	_curSheet = 0;
	_curContext = 0;
	_curVoice = 0;
	
	_curClef = 0;
	_curTimeSig = 0;
	_curKeySig = 0;
	_curBarline = 0;
	_curNote = 0;
	_curRest = 0;
}

CACanorusML::~CACanorusML() {
}

void CACanorusML::saveDocument(QTextStream& out, CADocument *doc) {
	int depth = 0;

	// CADocument
	QDomDocument dDoc("canorusml");
	
	// Add encoding
	dDoc.appendChild(dDoc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\" "));
	
	// Root node - <canorus-document>
	QDomElement dCanorusDocument = dDoc.createElement("canorus-document"); dDoc.appendChild(dCanorusDocument);
	// Add program version
	QDomElement dCanorusVersion = dDoc.createElement("canorus-version"); dCanorusDocument.appendChild(dCanorusVersion);
	dCanorusVersion.appendChild(dDoc.createTextNode(CANORUS_VERSION));
	
	// Document content node - <document>
	QDomElement dDocument = dDoc.createElement("document");
	dCanorusDocument.appendChild(dDocument);
	
	if (!doc->title().isEmpty())
		dDocument.setAttribute("title", doc->title());
	if (!doc->composer().isEmpty())
		dDocument.setAttribute("composer", doc->composer());
		
	for (int sheetIdx=0; sheetIdx < doc->sheetCount(); sheetIdx++) {
		// CASheet
		QDomElement dSheet = dDoc.createElement("sheet"); dDocument.appendChild(dSheet);
		dSheet.setAttribute("name", doc->sheetAt(sheetIdx)->name());
		
		for (int contextIdx=0; contextIdx < doc->sheetAt(sheetIdx)->contextCount(); contextIdx++) {
			// (CAContext)
			CAContext *c = doc->sheetAt(sheetIdx)->contextAt(contextIdx);
			
			switch (c->contextType()) {
				case CAContext::Staff:
					// CAStaff
					CAStaff *staff = (CAStaff*)c;
					QDomElement dStaff = dDoc.createElement("staff"); dSheet.appendChild(dStaff);
					dStaff.setAttribute("name", staff->name());
					dStaff.setAttribute("number-of-lines", staff->numberOfLines());
					
					for (int voiceIdx=0; voiceIdx < staff->voiceCount(); voiceIdx++) {
						//CAVoice
						CAVoice *v = staff->voiceAt(voiceIdx);
						QDomElement dVoice = dDoc.createElement("voice"); dStaff.appendChild(dVoice);
						dVoice.setAttribute("name", v->name());
						dVoice.setAttribute("midi-channel", v->midiChannel());
						dVoice.setAttribute("midi-program", v->midiProgram());
						dVoice.setAttribute("stem-direction", CANote::stemDirectionToString(v->stemDirection()));
						
						CACanorusML::writeVoice(dVoice, v);						
					}
					
					break;
			}
		}		
	}
	
	out << dDoc.toString();
}

void CACanorusML::writeVoice(QDomElement& dVoice, CAVoice* voice) {
	QDomDocument dDoc = dVoice.ownerDocument();
	for (int i=0; i<voice->musElementCount(); i++) {
		CAMusElement *curElt = voice->musElementAt(i);
		switch (curElt->musElementType()) {
			case CAMusElement::Note: {
				CANote *note = (CANote*)curElt;
				QDomElement dNote = dDoc.createElement("note"); dVoice.appendChild(dNote);
				dNote.setAttribute("playable-length", CAPlayable::playableLengthToString(note->playableLength()));
				dNote.setAttribute("pitch", note->pitch());
				dNote.setAttribute("accs", note->accidentals());
				if (note->stemDirection()!=CANote::StemPrefered)
					dNote.setAttribute("stem-direction", CANote::stemDirectionToString(note->stemDirection()));
				dNote.setAttribute("time-start", note->timeStart());
				dNote.setAttribute("time-length", note->timeLength());
				dNote.setAttribute("dotted", note->dotted());
				break;
			}
			case CAMusElement::Rest: {
				CARest *rest = (CARest*)curElt;
				QDomElement dRest = dDoc.createElement("rest"); dVoice.appendChild(dRest);
				dRest.setAttribute("playable-length", CAPlayable::playableLengthToString(rest->playableLength()));
				dRest.setAttribute("rest-type", CARest::restTypeToString(rest->restType()));
				dRest.setAttribute("time-start", rest->timeStart());
				dRest.setAttribute("time-length", rest->timeLength());
				dRest.setAttribute("dotted", rest->dotted());
				break;
			}
			case CAMusElement::Clef: {
				CAClef *clef = (CAClef*)curElt;
				QDomElement dClef = dDoc.createElement("clef"); dVoice.appendChild(dClef);
				dClef.setAttribute("clef-type", CAClef::clefTypeToString(clef->clefType()));
				dClef.setAttribute("time-start", clef->timeStart());
				break;
			}
			case CAMusElement::KeySignature: {
				CAKeySignature *key = (CAKeySignature*)curElt;
				QDomElement dKey = dDoc.createElement("key-signature"); dVoice.appendChild(dKey);
				dKey.setAttribute("key-signature-type", CAKeySignature::keySignatureTypeToString(key->keySignatureType()));
				
				if (key->keySignatureType()==CAKeySignature::MajorMinor || key->keySignatureType()==CAKeySignature::Modus) {
					dKey.setAttribute("accs", key->numberOfAccidentals());
					if (key->keySignatureType()==CAKeySignature::MajorMinor) {
						dKey.setAttribute("major-minor-gender", CAKeySignature::majorMinorGenderToString(key->majorMinorGender()));
					} else
					if (key->keySignatureType()==CAKeySignature::Modus) {
						dKey.setAttribute("modus", CAKeySignature::modusToString(key->modus()));
					}
					// TODO: Custom accidentals
				}
				
				dKey.setAttribute("time-start", key->timeStart());
				break;
			}
			case CAMusElement::TimeSignature: {
				CATimeSignature *time = (CATimeSignature*)curElt;
				QDomElement dTime = dDoc.createElement("time-signature"); dVoice.appendChild(dTime);
				dTime.setAttribute("time-signature-type", CATimeSignature::timeSignatureTypeToString(time->timeSignatureType()));
				dTime.setAttribute("beats", time->beats());
				dTime.setAttribute("beat", time->beat());
				dTime.setAttribute("time-start", time->timeStart());
				break;
			}
			case CAMusElement::Barline: {
				CABarline *barline = (CABarline*)curElt;
				QDomElement dBarline = dDoc.createElement("barline"); dVoice.appendChild(dBarline);
				dBarline.setAttribute("barlineType", CABarline::barlineTypeToString(barline->barlineType()));
				dBarline.setAttribute("time-start", barline->timeStart());
				break;
			}
		}
	}
}

CADocument* CACanorusML::openDocument(QXmlInputSource* in, CAMainWin *mainWin) {
	QXmlSimpleReader reader;
	CACanorusML *canHandler = new CACanorusML(mainWin);
	reader.setContentHandler(canHandler);
	reader.parse(in);
	
	CADocument *doc = canHandler->document();
	delete canHandler;
	return doc;
}

/** DEPRECATED - should be moved to LilyPond parser */
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
				voiceString += QString("<key type=\"") + key->majorMinorGenderML() + "\">";
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
	if (qName == "document") {
		// CADocument
		_document = new CADocument();
	} else if (qName == "sheet") {
		// CASheet	
		QString sheetName = attributes.value("name");
		if (!(_curSheet = _document->sheet(sheetName))) {	//if the document doesn't contain the sheet with the given name, create a new sheet and add it to the document. Otherwise, just set the current sheet to the found one and leave
			if (sheetName.isEmpty())
				sheetName = QString("Sheet") + " " + QString::number(_document->sheetCount()+1);
			_curSheet = new CASheet(sheetName, _document);
			
			_document->addSheet(_curSheet);
			_mainWin->addSheet(_curSheet);
		}
	} else if (qName == "staff") {
		// CAStaff
		QString staffName = attributes.value("name");
		if (!_curSheet) {
			_errorMsg = "The sheet where to add the staff doesn't exist yet!";
			return false;
		}
		
		if (!(_curContext = _curSheet->context(staffName))) {	//if the sheet doesn't contain the staff with the given name, create a new sheet and add it to the document. Otherwise, just set the current staff to the found one and leave
			if (staffName.isEmpty())
				staffName = QString("Staff") + " " + QString::number(_curSheet->staffCount()+1);
			_curContext = new CAStaff(_curSheet, staffName, attributes.value("number-of-lines").toInt());
		}
		_curSheet->addContext(_curContext);
	} else if (qName == "voice") {
		// CAVoice
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
			_curVoice = new CAVoice((CAStaff*)_curContext, voiceName);
			if (!attributes.value("stemDirection").isEmpty())
				_curVoice->setStemDirection(CANote::stemDirectionFromString(attributes.value("stem-direction")));
			((CAStaff*)_curContext)->addVoice(_curVoice);
		}
	}
	else if (qName == "clef") {
		// CAClef
		_curClef = new CAClef(CAClef::clefTypeFromString(attributes.value("clef-type")),
		                                                 _curVoice->staff(),
		                                                 attributes.value("time-start").toInt()
		                                                );
	}
	else if (qName == "time-signature") {
		// CATimeSignature
		_curTimeSig = new CATimeSignature(attributes.value("beats").toInt(),
		                                  attributes.value("beat").toInt(),
		                                  _curVoice->staff(),
		                                  attributes.value("time-start").toInt(),
		                                  CATimeSignature::timeSignatureTypeFromString(attributes.value("time-signature-type"))
		                                 );
	} else if (qName == "key-signature") {
		// CAKeySignature
		_curKeySig = new CAKeySignature(CAKeySignature::keySignatureTypeFromString(attributes.value("key-signature-type")),
		                                (char)attributes.value("accs").toInt(),
		                                CAKeySignature::majorMinorGenderFromString(attributes.value("major-minor-gender")),
		                                _curVoice->staff(),
		                                attributes.value("time-start").toInt()
		                               );
		if (_curKeySig->keySignatureType()==CAKeySignature::MajorMinor) {
			_curKeySig->setMajorMinorGender(CAKeySignature::majorMinorGenderFromString(attributes.value("major-minor-gender")));
		}
		else if (_curKeySig->keySignatureType()==CAKeySignature::Modus) {
			_curKeySig->setModus(CAKeySignature::modusFromString(attributes.value("modus")));
		}
	} else if (qName == "barline") {
		// CABarline
		_curBarline = new CABarline(CABarline::barlineTypeFromString(attributes.value("barline-type")),
	                                _curVoice->staff(),
	                                attributes.value("time-start").toInt()
	                               );
	} else if (qName == "note") {
		// CANote
		_curNote = new CANote(CAPlayable::playableLengthFromString(attributes.value("playable-length")),
		                      _curVoice,
		                      attributes.value("pitch").toInt(),
		                      (char)attributes.value("accs").toInt(),
		                      attributes.value("time-start").toInt(),
		                      attributes.value("dotted").toInt()
		                     );
		if (!attributes.value("stem-direction").isEmpty())
			_curNote->setStemDirection(CANote::stemDirectionFromString(attributes.value("stem-direction")));
	} else if (qName == "rest") {
		// CARest
		_curRest = new CARest(CARest::restTypeFromString(attributes.value("rest-type")),
		                      CAPlayable::playableLengthFromString(attributes.value("playable-length")),
		                      _curVoice,
		                      attributes.value("time-start").toInt(),
		                      attributes.value("dotted").toInt()
		                     );
	}
	
	_depth.push(qName);
	return true;
}

bool CACanorusML::endElement(const QString& namespaceURI, const QString& localName, const QString& qName) {
	if (qName == "canorus-version") {
		// version of Canorus which saved the document
		_version = _cha;
	} else if (qName == "document") {
		//fix voice errors like shared voice elements not being present in both voices etc.
		for (int i=0; _document && i<_document->sheetCount(); i++) {
			for (int j=0; j<_document->sheetAt(i)->staffCount(); j++) {
				_document->sheetAt(i)->staffAt(j)->fixVoiceErrors();
			}
		}
	} else if (qName == "sheet") {
		// CASheet
		_curSheet = 0;			
	} else if (qName == "staff") {
		// CAStaff
		_curContext = 0;
	} else if (qName == "voice") {
		// CAVoice
		_curVoice = 0;
	}
	//Every voice *must* contain signs on their own (eg. a clef is placed in all voices, not just the first one).
	//The following code finds a sign with the same properties at the same time in other voices. If such a sign exists, only place a pointer to this sign in the current voice. Otherwise, add a sign to all the voices read so far.
	else if (qName == "clef") {
		// CAClef
		if (!_curContext || !_curVoice || _curContext->contextType()!=CAContext::Staff) {
			return false;
		}
		
		//lookup an element with the same type at the same time
		QList<CAMusElement*> foundElts = ((CAStaff*)_curContext)->getEltByType(CAMusElement::Clef, _curVoice->lastTimeEnd());
		CAMusElement *sign=0;
		for (int i=0; i<foundElts.size(); i++) {
			if (!foundElts[i]->compare(_curClef))	//element has exactly the same properties
				if (!_curVoice->contains(foundElts[i]))	{ //element isn't present in the voice yet
					sign = foundElts[i];
					break;
				}
		}
			
		if (!sign) {
			//the element doesn't exist yet - add it
			_curVoice->staff()->insertSignAfter(_curClef, _curVoice->musElementCount()?_curVoice->lastMusElement():0, true);
		} else {
			//the element was found, insert only a reference to the current voice
			_curVoice->appendMusElement(sign);
			delete _curClef; _curClef = 0; 
		}
	} else if (qName == "key-signature") {
		// CAKeySignature
		if (!_curContext || !_curVoice || _curContext->contextType()!=CAContext::Staff) {
			return false;
		}
		
		//lookup an element with the same type at the same time
		QList<CAMusElement*> foundElts = ((CAStaff*)_curContext)->getEltByType(CAMusElement::KeySignature, _curVoice->lastTimeEnd());
		CAMusElement *sign=0;
		for (int i=0; i<foundElts.size(); i++) {
			if (!foundElts[i]->compare(_curKeySig))	//element has exactly the same properties
				if (!_curVoice->contains(foundElts[i]))	{ //element isn't present in the voice yet
					sign = foundElts[i];
					break;
				}
		}
		
		if (!sign) {
			//the element doesn't exist yet - add it
			_curVoice->staff()->insertSignAfter(_curKeySig, _curVoice->musElementCount()?_curVoice->lastMusElement():0, true);
		} else {
			//the element was found, insert only a reference to the current voice
			_curVoice->appendMusElement(sign);
			delete _curKeySig; _curKeySig = 0;
		}
	} else if (qName == "time-signature") {
		// CATimeSignature
		if (!_curContext || !_curVoice || _curContext->contextType()!=CAContext::Staff) {
			return false;
		}
		
		//lookup an element with the same type at the same time
		QList<CAMusElement*> foundElts = ((CAStaff*)_curContext)->getEltByType(CAMusElement::TimeSignature, _curVoice->lastTimeEnd());
		CAMusElement *sign=0;
		for (int i=0; i<foundElts.size(); i++) {
			if (!foundElts[i]->compare(_curTimeSig))	//element has exactly the same properties
				if (!_curVoice->contains(foundElts[i]))	{ //element isn't present in the voice yet
					sign = foundElts[i];
					break;
				}
		}
			
		if (!sign) {
			//the element doesn't exist yet - add it
			_curVoice->staff()->insertSignAfter(_curTimeSig, _curVoice->musElementCount()?_curVoice->lastMusElement():0, true);
		} else {
			//the element was found, insert only a reference to the current voice
			_curVoice->appendMusElement(sign);
			delete _curTimeSig; _curTimeSig = 0;
		}
	} else if (qName == "barline") {
		// CABarline
		if (!_curContext || !_curVoice || _curContext->contextType()!=CAContext::Staff) {
			return false;
		}
		
		//lookup an element with the same type at the same time
		QList<CAMusElement*> foundElts = ((CAStaff*)_curContext)->getEltByType(CAMusElement::Barline, _curVoice->lastTimeEnd());
		CAMusElement *sign=0;
		for (int i=0; i<foundElts.size(); i++) {
			if (!foundElts[i]->compare(_curBarline))	//element has exactly the same properties
				if (!_curVoice->contains(foundElts[i]))	{ //element isn't present in the voice yet
					sign = foundElts[i];
					break;
				}
		}
			
		if (!sign) {
			//the element doesn't exist yet - add it
			_curVoice->staff()->insertSignAfter(_curBarline, _curVoice->musElementCount()?_curVoice->lastMusElement():0, true);
		} else {
			//the element was found, insert only a reference to the current voice
			_curVoice->appendMusElement(sign);
			delete _curBarline; _curBarline = 0;
		}
	} else if (qName == "note") {
		// CANote
		_curVoice->appendMusElement(_curNote);
		_curNote = 0;
	} else if (qName == "rest") {
		// CARest
		_curVoice->appendMusElement(_curRest);
		_curRest = 0;		
	}
	
	_cha="";
	_depth.pop();
	return true;
}

bool CACanorusML::characters(const QString& ch) {
	_cha = ch;
	
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
