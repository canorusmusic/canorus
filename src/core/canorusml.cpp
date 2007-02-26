/*
 * Copyright (c) 2006, Matevž Jekovec, Georg Rudolph, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 *
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
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

/*!
	\class CACanorusML
	\brief Class used for writing/opening a Canorus document.
	
	This class is used for saving and opening a native Canorus document stored in
	CanorusML format. The class inherits QXmlDefaultHandler and the class itself is
	an XML parser.
	
	For writing or opening the document, simply use the static methods saveDocument()
	and openDocument(). saveDocument() doesn't need XML parser, so only the static
	method is used. openDocument() needs an XML parser and creates a new CACanorusML
	class and use it as a parser.
	
	\sa CADocument
*/

/*!
	Creates a CanorusML parser class
*/
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

/*!
	Destroys parser.
*/
CACanorusML::~CACanorusML() {
}

/*!
	Saves the document.
	It uses DOM object internally for writing the XML output.
	
	\sa openDocument()
*/
void CACanorusML::saveDocument(CADocument *doc, QTextStream& out) {
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

/*!
	Used for writing the voice node in XML output.
	It uses DOM object internally for writing the XML output.
	This method is usually called by saveDocument().
	
	\sa saveDocument()
*/
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
					//! \todo Custom accidentals in key signature saving -Matevz
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
				dBarline.setAttribute("barline-type", CABarline::barlineTypeToString(barline->barlineType()));
				dBarline.setAttribute("time-start", barline->timeStart());
				break;
			}
		}
	}
}

/*!
	Opens a CanorusML source \a in and creates a document out of it.
	\a mainWin is needed for any UI settings stored in the file (the last viewports
	positions, current sheet etc.).
	CACanorusML uses SAX parser for reading.
	
	\todo It would probably be better for us to use DOM parser for reading as well in the
	future. -Matevz
	
	\sa saveDocument()
*/
CADocument* CACanorusML::openDocument(QXmlInputSource* in, CAMainWin *mainWin) {
	QXmlSimpleReader reader;
	CACanorusML *canHandler = new CACanorusML(mainWin);
	reader.setContentHandler(canHandler);
	reader.parse(in);
	
	CADocument *doc = canHandler->document();
	delete canHandler;
	return doc;
}

/*!
	This method should be called when a critical error occurs while parsing the XML source.
	
	\sa startElement(), endElement()
*/
bool CACanorusML::fatalError (const QXmlParseException & exception) {
	qWarning() << "Fatal error on line " << exception.lineNumber()
		<< ", column " << exception.columnNumber() << ": "
		<< exception.message() << "\n\nParser message:\n" << _errorMsg;
	
	return false;
}

/*!
	This function is called automatically by Qt SAX parser while reading the CanorusML
	source. This function is called when a new node is opened. It already reads node
	attributes.
	
	The function returns true, if the node was successfully recognized and parsed;
	otherwise false.
	
	\sa endElement()
*/
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
			if (!attributes.value("stem-direction").isEmpty())
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

/*!
	This function is called automatically by Qt SAX parser while reading the CanorusML
	source. This function is called when a node has been closed (</nodeName>). Attributes
	for closed notes are usually not set in CanorusML format. That's why we need to store
	local node attributes (set when the node is opened) each time.
	
	The function returns true, if the node was successfully recognized and parsed;
	otherwise false.
	
	\sa startElement()
*/
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
	// Every voice *must* contain signs on their own (eg. a clef is placed in all voices, not just the first one).
	// The following code finds a sign with the same properties at the same time in other voices. If such a sign exists, only place a pointer to this sign in the current voice. Otherwise, add a sign to all the voices read so far.
	else if (qName == "clef") {
		// CAClef
		if (!_curContext || !_curVoice || _curContext->contextType()!=CAContext::Staff) {
			return false;
		}
		
		// lookup an element with the same type at the same time
		QList<CAMusElement*> foundElts = ((CAStaff*)_curContext)->getEltByType(CAMusElement::Clef, _curVoice->lastTimeEnd());
		CAMusElement *sign=0;
		for (int i=0; i<foundElts.size(); i++) {
			if (!foundElts[i]->compare(_curClef))	// element has exactly the same properties
				if (!_curVoice->contains(foundElts[i]))	{ // element isn't present in the voice yet
					sign = foundElts[i];
					break;
				}
		}
		
		if (!sign) {
			// the element doesn't exist yet - add it
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
		
		// lookup an element with the same type at the same time
		QList<CAMusElement*> foundElts = ((CAStaff*)_curContext)->getEltByType(CAMusElement::KeySignature, _curVoice->lastTimeEnd());
		CAMusElement *sign=0;
		for (int i=0; i<foundElts.size(); i++) {
			if (!foundElts[i]->compare(_curKeySig))	// element has exactly the same properties
				if (!_curVoice->contains(foundElts[i]))	{ // element isn't present in the voice yet
					sign = foundElts[i];
					break;
				}
		}
		
		if (!sign) {
			// the element doesn't exist yet - add it
			_curVoice->staff()->insertSignAfter(_curKeySig, _curVoice->musElementCount()?_curVoice->lastMusElement():0, true);
		} else {
			// the element was found, insert only a reference to the current voice
			_curVoice->appendMusElement(sign);
			delete _curKeySig; _curKeySig = 0;
		}
	} else if (qName == "time-signature") {
		// CATimeSignature
		if (!_curContext || !_curVoice || _curContext->contextType()!=CAContext::Staff) {
			return false;
		}
		
		// lookup an element with the same type at the same time
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
			// the element doesn't exist yet - add it
			_curVoice->staff()->insertSignAfter(_curTimeSig, _curVoice->musElementCount()?_curVoice->lastMusElement():0, true);
		} else {
			// the element was found, insert only a reference to the current voice
			_curVoice->appendMusElement(sign);
			delete _curTimeSig; _curTimeSig = 0;
		}
	} else if (qName == "barline") {
		// CABarline
		if (!_curContext || !_curVoice || _curContext->contextType()!=CAContext::Staff) {
			return false;
		}
		
		// lookup an element with the same type at the same time
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
			// the element doesn't exist yet - add it
			_curVoice->staff()->insertSignAfter(_curBarline, _curVoice->musElementCount()?_curVoice->lastMusElement():0, true);
		} else {
			// the element was found, insert only a reference to the current voice
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

/*!
	Stores the characters between the greater-lesser signs while parsing the XML file.
	This is usually needed for getting the property values stored not as node attributes,
	but between greater-lesser signs.
	
	eg.
	\code
		<length>127</length>
	\endcode
	Would set _cha value to "127".
	
	\sa startElement(), endElement()
*/
bool CACanorusML::characters(const QString& ch) {
	_cha = ch;
	
	return true;
}

/*!
	\fn CACanorusML::document()
	Returns the newly created document when reading the XML file.
*/

/*!
	\var CACanorusML::_cha
	Current characters being read using characters() method between the greater/lesser
	separators in XML file.
	
	\sa characters()
*/

/*!
	\var CACanorusML::_depth
	Stack which represents the current depth of the document while SAX parsing. It contains
	the tag names as the values.
	
	\sa startElement(), endElement()
*/

/*!
	\var CACanorusML::_errorMsg
	The error message content stored as QString, if the error happens.
	
	\sa fatalError()
*/

/*!
	\var CACanorusML::_version
	Document program version - which Canorus did save the file.
	
	\sa startElement(), endElement()
*/

/*!
	\var CACanorusML::_mainWin
	Pointer to the main window of the application - needed to rebuild the UI.
	
	\sa CAMainWin
*/

/*!
	\var CACanorusML::_document
	Pointer to the document being read.
	
	\sa CADocument
*/
