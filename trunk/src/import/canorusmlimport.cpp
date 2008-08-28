/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QDebug>
#include <QIODevice>

#include "import/canorusmlimport.h"

#include "core/document.h"
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

CACanorusMLImport::CACanorusMLImport( QTextStream *stream )
 : CAImport(stream) {
	initCanorusMLImport();
}

CACanorusMLImport::CACanorusMLImport( const QString stream )
 : CAImport(stream) {
	initCanorusMLImport();
}

CACanorusMLImport::~CACanorusMLImport() {
}

void CACanorusMLImport::initCanorusMLImport() {
	_document   = 0;
	_curSheet   = 0;
	_curContext = 0;
	_curVoice   = 0;

	_curMusElt       = 0;
	_curMark         = 0;
	_curClef         = 0;
	_curTimeSig      = 0;
	_curKeySig       = 0;
	_curBarline      = 0;
	_curNote         = 0;
	_curRest         = 0;
	_curTie          = 0;
	_curSlur         = 0;
	_curPhrasingSlur = 0;
	_curTuplet       = 0;
}

/*!
	Opens a CanorusML source \a in and creates a document out of it.
	\a mainWin is needed for any UI settings stored in the file (the last viewports
	positions, current sheet etc.).
	CACanorusML uses SAX parser for reading.
*/
CADocument* CACanorusMLImport::importDocumentImpl() {
	QIODevice *device = stream()->device();
	QXmlInputSource *src;
	if(device)
		src = new QXmlInputSource( device );
	else {
		src = new QXmlInputSource();
		src->setData( *stream()->string() );
	}
	QXmlSimpleReader *reader = new QXmlSimpleReader();
	reader->setContentHandler( this );
	reader->parse( src );

	delete reader;
	delete src;

	return document();
}

/*!
	This method should be called when a critical error occurs while parsing the XML source.

	\sa startElement(), endElement()
*/
bool CACanorusMLImport::fatalError ( const QXmlParseException & exception ) {
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
bool CACanorusMLImport::startElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& attributes ) {
	if (qName == "document") {
		// CADocument
		_document = new CADocument();
		_document->setTitle( attributes.value("title") );
		_document->setSubtitle( attributes.value("subtitle") );
		_document->setComposer( attributes.value("composer") );
		_document->setArranger( attributes.value("arranger") );
		_document->setPoet( attributes.value("poet") );
		_document->setTextTranslator( attributes.value("text-translator") );
		_document->setCopyright( attributes.value("copyright") );
		_document->setDedication( attributes.value("dedication") );
		_document->setComments( attributes.value("comments") );

		_document->setDateCreated( QDateTime::fromString( attributes.value("date-created"), Qt::ISODate ) );
		_document->setDateLastModified( QDateTime::fromString( attributes.value("date-last-modified"), Qt::ISODate ) );
		_document->setTimeEdited( attributes.value("time-edited").toUInt() );

	} else if (qName == "sheet") {
		// CASheet
		QString sheetName = attributes.value("name");
		if (!(_curSheet = _document->sheet(sheetName))) {	//if the document doesn't contain the sheet with the given name, create a new sheet and add it to the document. Otherwise, just set the current sheet to the found one and leave
			if (sheetName.isEmpty())
				sheetName = QObject::tr("Sheet%1").arg(_document->sheetCount()+1);
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
				staffName = QObject::tr("Staff%1").arg(_curSheet->staffCount()+1);
			_curContext = new CAStaff( staffName, _curSheet, attributes.value("number-of-lines").toInt());
		}
		_curSheet->addContext(_curContext);
	} else if (qName == "lyrics-context") {
		// CALyricsContext
		QString lcName = attributes.value("name");
		if (!_curSheet) {
			_errorMsg = "The sheet where to add the lyrics context doesn't exist yet!";
			return false;
		}

		if (!(_curContext = _curSheet->context(lcName))) {	//if the sheet doesn't contain the context with the given name, create a new sheet and add it to the document. Otherwise, just set the current staff to the found one and leave
			if (lcName.isEmpty())
				lcName = QObject::tr("Lyrics Context %1").arg(_curSheet->contextCount()+1);
			_curContext = new CALyricsContext( lcName, attributes.value("stanza-number").toInt(), _curSheet );

			// voices are not neccesseraly completely read - store indices of the voices internally and then assign them at the end
			if (!attributes.value("associated-voice-idx").isEmpty())
				_lcMap[static_cast<CALyricsContext*>(_curContext)] = attributes.value("associated-voice-idx").toInt();
		}
		_curSheet->addContext(_curContext);
	} else if (qName == "function-mark-context" || qName == "function-marking-context") {
		// CAFunctionMarkContext
		QString fmcName = attributes.value("name");
		if (!_curSheet) {
			_errorMsg = "The sheet where to add the function mark context doesn't exist yet!";
			return false;
		}

		if (!(_curContext = _curSheet->context(fmcName))) {	//if the sheet doesn't contain the context with the given name, create a new sheet and add it to the document. Otherwise, just set the current staff to the found one and leave
			if (fmcName.isEmpty())
				fmcName = QObject::tr("Function Mark Context %1").arg(_curSheet->contextCount()+1);
			_curContext = new CAFunctionMarkContext( fmcName, _curSheet );
		}
		_curSheet->addContext(_curContext);
	} else if (qName == "voice") {
		// CAVoice
		QString voiceName = attributes.value("name");
		if (!_curContext) {
			_errorMsg = "The context where the voice " + voiceName + " should be added doesn't exist yet!";
			return false;
		} else if (_curContext->contextType() != CAContext::Staff) {
			_errorMsg = "The context type which contains voice " + voiceName + " isn't staff!";
			return false;
		}

		CAStaff *staff = static_cast<CAStaff*>(_curContext);
		if (!(_curVoice = staff->voiceByName(voiceName))) {	// if the staff doesn't contain the voice with the given name, create a new voice and add it to the document. Otherwise, just set the current voice to the found one and leave
			int voiceNumber = staff->voiceCount()+1;

			if (voiceName.isEmpty())
				voiceName = QObject::tr("Voice%1").arg( voiceNumber );

			CANote::CAStemDirection stemDir = CANote::StemNeutral;
			if (!attributes.value("stem-direction").isEmpty())
				stemDir = CANote::stemDirectionFromString(attributes.value("stem-direction"));

			_curVoice = new CAVoice( voiceName, staff, stemDir, voiceNumber );
			staff->addVoice( _curVoice );
		}
	}
	else if (qName == "clef") {
		// CAClef
		_curClef = new CAClef( CAClef::clefTypeFromString(attributes.value("clef-type")),
		                       attributes.value("c1").toInt(),
		                       _curVoice->staff(),
		                       attributes.value("time-start").toInt(),
		                       attributes.value("offset").toInt()
		);
		_curMusElt = _curClef;
	}
	else if (qName == "time-signature") {
		// CATimeSignature
		_curTimeSig = new CATimeSignature( attributes.value("beats").toInt(),
		                                   attributes.value("beat").toInt(),
		                                   _curVoice->staff(),
		                                   attributes.value("time-start").toInt(),
		                                   CATimeSignature::timeSignatureTypeFromString(attributes.value("time-signature-type"))
		);
		_curMusElt = _curTimeSig;
	} else if (qName == "key-signature") {
		// CAKeySignature
		CAKeySignature::CAKeySignatureType type = CAKeySignature::keySignatureTypeFromString(attributes.value("key-signature-type"));
		switch (type) {
		case CAKeySignature::MajorMinor: {
			_curKeySig = new CAKeySignature( CADiatonicKey(),
			                                 _curVoice->staff(),
					                         attributes.value("time-start").toInt()
					                       );
			break;
		}
		case CAKeySignature::Modus: {
			_curKeySig = new CAKeySignature( CAKeySignature::modusFromString(attributes.value("modus")),
						                     _curVoice->staff(),
								             attributes.value("time-start").toInt()
								           );
			break;
		}
		}

		_curMusElt = _curKeySig;
	} else if (qName == "barline") {
		// CABarline
		_curBarline = new CABarline(CABarline::barlineTypeFromString(attributes.value("barline-type")),
	                                _curVoice->staff(),
	                                attributes.value("time-start").toInt()
	                               );
		_curMusElt = _curBarline;
	} else if (qName == "note") {
		// CANote
		if ( _version.startsWith("0.5") ) {
		_curNote = new CANote( CADiatonicPitch( attributes.value("pitch").toInt(), attributes.value("accs").toInt() ),
		                       CAPlayableLength( CAPlayableLength::musicLengthFromString(attributes.value("playable-length")), attributes.value("dotted").toInt()),
		                      _curVoice,
		                      attributes.value("time-start").toInt(),
		                      attributes.value("time-length").toInt()
		                     );
		} else {
			_curNote = new CANote( CADiatonicPitch(),
			                       CAPlayableLength(),
			                       _curVoice,
			                       attributes.value("time-start").toInt(),
			                       attributes.value("time-length").toInt()
			                     );
		}

		if (!attributes.value("stem-direction").isEmpty()) {
			_curNote->setStemDirection(CANote::stemDirectionFromString(attributes.value("stem-direction")));
		}

		if (_curTuplet) {
			_curNote->setTuplet( _curTuplet );
			_curTuplet->addNote( _curNote );
		}

		_curMusElt = _curNote;

	} else if (qName == "tie") {
		_curTie = new CASlur( CASlur::TieType, CASlur::SlurPreferred, _curNote->staff(), _curNote, 0 );
		_curNote->setTieStart( _curTie );
		if (!attributes.value("slur-style").isEmpty())
			_curTie->setSlurStyle( CASlur::slurStyleFromString( attributes.value("slur-style") ) );
		if (!attributes.value("slur-direction").isEmpty())
			_curTie->setSlurDirection( CASlur::slurDirectionFromString( attributes.value("slur-direction") ) );
		_curMusElt = _curTie;
	} else if (qName == "slur-start") {
		_curSlur = new CASlur( CASlur::SlurType, CASlur::SlurPreferred, _curNote->staff(), _curNote, 0 );
		_curNote->setSlurStart( _curSlur );
		if (!attributes.value("slur-style").isEmpty())
			_curSlur->setSlurStyle( CASlur::slurStyleFromString( attributes.value("slur-style") ) );
		if (!attributes.value("slur-direction").isEmpty())
			_curSlur->setSlurDirection( CASlur::slurDirectionFromString( attributes.value("slur-direction") ) );
		_curMusElt = _curSlur;
	} else if (qName == "slur-end") {
		_curNote->setSlurEnd( _curSlur );
		_curSlur->setNoteEnd( _curNote );
		_curSlur = 0;
		_curMusElt = _curSlur;
	} else if (qName == "phrasing-slur-start") {
		_curPhrasingSlur = new CASlur( CASlur::PhrasingSlurType, CASlur::SlurPreferred, _curNote->staff(), _curNote, 0 );
		_curNote->setPhrasingSlurStart( _curPhrasingSlur );
		if (!attributes.value("slur-style").isEmpty())
			_curPhrasingSlur->setSlurStyle( CASlur::slurStyleFromString( attributes.value("slur-style") ) );
		if (!attributes.value("slur-direction").isEmpty())
			_curPhrasingSlur->setSlurDirection( CASlur::slurDirectionFromString( attributes.value("slur-direction") ) );
		_curMusElt = _curPhrasingSlur;
	} else if (qName == "phrasing-slur-end") {
		_curNote->setPhrasingSlurEnd( _curPhrasingSlur );
		_curPhrasingSlur->setNoteEnd( _curNote );
		_curPhrasingSlur = 0;
		_curMusElt = _curPhrasingSlur;
	} else if ( qName == "tuplet" ) {
		_curTuplet = new CATuplet( attributes.value("number").toInt(), attributes.value("actual-number").toInt() );
	} else if (qName == "rest") {
		// CARest
		if ( _version.startsWith("0.5") ) {
			_curRest = new CARest( CARest::restTypeFromString(attributes.value("rest-type")),
			                       CAPlayableLength( CAPlayableLength::musicLengthFromString(attributes.value("playable-length")), attributes.value("dotted").toInt()),
			                      _curVoice,
			                      attributes.value("time-start").toInt(),
			                      attributes.value("time-length").toInt()
			                     );
		} else {
			_curRest = new CARest( CARest::restTypeFromString(attributes.value("rest-type")),
					               CAPlayableLength(),
					               _curVoice,
					               attributes.value("time-start").toInt(),
					               attributes.value("time-length").toInt()
					             );
		}

		if (_curTuplet) {
			_curRest->setTuplet( _curTuplet );
			_curTuplet->addNote( _curRest );
		}

		_curMusElt = _curRest;
	} else if (qName == "syllable") {
		// CASyllable
		CASyllable *s = new CASyllable(
			attributes.value("text"),
			attributes.value("hyphen")=="1",
			attributes.value("melisma")=="1",
			static_cast<CALyricsContext*>(_curContext),
			attributes.value("time-start").toInt(),
			attributes.value("time-length").toInt()
		);

		static_cast<CALyricsContext*>(_curContext)->addSyllable(s);
		if (!attributes.value("associated-voice-idx").isEmpty())
			_syllableMap[s] = attributes.value("associated-voice-idx").toInt();
		_curMusElt = s;
	} else if (qName == "function-mark" || _version.startsWith("0.5") && qName == "function-marking") {
		// CAFunctionMark
		CAFunctionMark *f =
			new CAFunctionMark(
				CAFunctionMark::functionTypeFromString(attributes.value("function")),
				(attributes.value("minor")=="1"?true:false),
				(_version.startsWith("0.5")?(attributes.value("key").isEmpty()?"C":attributes.value("key")):CADiatonicKey()),
				static_cast<CAFunctionMarkContext*>(_curContext),
				attributes.value("time-start").toInt(),
				attributes.value("time-length").toInt(),
				CAFunctionMark::functionTypeFromString(attributes.value("chord-area")),
				(attributes.value("chord-area-minor")=="1"?true:false),
				CAFunctionMark::functionTypeFromString(attributes.value("tonic-degree")),
				(attributes.value("tonic-degree-minor")=="1"?true:false),
				"",
				(attributes.value("ellipse")=="1"?true:false)
			);

		static_cast<CAFunctionMarkContext*>(_curContext)->addFunctionMark(f);
		_curMusElt = f;
	} else if (qName == "mark") {
		// CAMark and subvariants
		importMark( attributes );
	} else if (qName == "playable-length") {
		_curPlayableLength = CAPlayableLength( CAPlayableLength::musicLengthFromString(attributes.value("music-length")), attributes.value("dotted").toInt() );
	} else if (qName == "diatonic-pitch") {
		_curDiatonicPitch = CADiatonicPitch( attributes.value("note-name").toInt(), attributes.value("accs").toInt() );
	} else if (qName == "diatonic-key") {
		_curDiatonicKey = CADiatonicKey( CADiatonicPitch(), CADiatonicKey::genderFromString(attributes.value("gender")) );
	}

	_depth.push(qName);
	return true;
}

/*!
	This function is called automatically by Qt SAX parser while reading the CanorusML
	source. This function is called when a node has been closed (\</nodeName\>). Attributes
	for closed notes are usually not set in CanorusML format. That's why we need to store
	local node attributes (set when the node is opened) each time.

	The function returns true, if the node was successfully recognized and parsed;
	otherwise false.

	\sa startElement()
*/
bool CACanorusMLImport::endElement( const QString& namespaceURI, const QString& localName, const QString& qName ) {
	if (qName == "canorus-version") {
		// version of Canorus which saved the document
		_version = _cha;
	} else if (qName == "document") {
		//fix voice errors like shared voice elements not being present in both voices etc.
		for (int i=0; _document && i<_document->sheetCount(); i++) {
			for (int j=0; j<_document->sheetAt(i)->staffCount(); j++) {
				_document->sheetAt(i)->staffAt(j)->synchronizeVoices();
			}
		}
	} else if (qName == "sheet") {
		// CASheet
		QList<CAVoice*> voices = _curSheet->voiceList();
		QList<CALyricsContext*> lcs = _lcMap.keys();
		for (int i=0; i<lcs.size(); i++) // assign voices from voice indices
			lcs.at(i)->setAssociatedVoice( voices.at(_lcMap[lcs[i]]) );

		QList<CASyllable*> syllables = _syllableMap.keys();
		for (int i=0; i<syllables.size(); i++) // assign voices from voice indices
			syllables.at(i)->setAssociatedVoice( voices.at(_syllableMap[syllables[i]]) );

		_lcMap.clear();
		_syllableMap.clear();
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
		QList<CAMusElement*> foundElts = static_cast<CAStaff*>(_curContext)->getEltByType(CAMusElement::Clef, _curClef->timeStart());
		CAMusElement *sign=0;
		for (int i=0; i<foundElts.size(); i++) {
			if (!foundElts[i]->compare(_curClef))	      // element has exactly the same properties
				if (!_curVoice->contains(foundElts[i]))	{ // if such an element already exists, it means there are two different with the same timestart
					sign = foundElts[i];
					break;
				}
		}

		if (!sign) {
			// the element doesn't exist yet - add it to all the voices
			_curVoice->append( _curClef );
		} else {
			//the element was found, insert only a reference to the current voice
			_curVoice->append( sign );
			delete _curClef; _curClef = 0;
		}
	} else if (qName == "key-signature") {
		// CAKeySignature
		if (!_curContext || !_curVoice || _curContext->contextType()!=CAContext::Staff) {
			return false;
		}

		switch (_curKeySig->keySignatureType()) {
		case CAKeySignature::MajorMinor:
			_curKeySig->setDiatonicKey( _curDiatonicKey );
			break;
		}

		// lookup an element with the same type at the same time
		QList<CAMusElement*> foundElts = static_cast<CAStaff*>(_curContext)->getEltByType(CAMusElement::KeySignature, _curKeySig->timeStart());
		CAMusElement *sign=0;
		for (int i=0; i<foundElts.size(); i++) {
			if (!foundElts[i]->compare(_curKeySig))	      // element has exactly the same properties
				if (!_curVoice->contains(foundElts[i]))	{ // if such an element already exists, it means there are two different with the same timestart
					sign = foundElts[i];
					break;
				}
		}

		if (!sign) {
			// the element doesn't exist yet - add it to all the voices
			_curVoice->append( _curKeySig );
		} else {
			// the element was found, insert only a reference to the current voice
			_curVoice->append( sign );
			delete _curKeySig; _curKeySig = 0;
		}
	} else if (qName == "time-signature") {
		// CATimeSignature
		if (!_curContext || !_curVoice || _curContext->contextType()!=CAContext::Staff) {
			return false;
		}

		// lookup an element with the same type at the same time
		QList<CAMusElement*> foundElts = static_cast<CAStaff*>(_curContext)->getEltByType(CAMusElement::TimeSignature, _curTimeSig->timeStart());
		CAMusElement *sign=0;
		for (int i=0; i<foundElts.size(); i++) {
			if (!foundElts[i]->compare(_curTimeSig))	  // element has exactly the same properties
				if (!_curVoice->contains(foundElts[i]))	{ // if such an element already exists, it means there are two different with the same timestart
					sign = foundElts[i];
					break;
				}
		}

		if (!sign) {
			// the element doesn't exist yet - add it to all the voices
			_curVoice->append( _curTimeSig );
		} else {
			// the element was found, insert only a reference to the current voice
			_curVoice->append( sign );
			delete _curTimeSig; _curTimeSig = 0;
		}
	} else if (qName == "barline") {
		// CABarline
		if (!_curContext || !_curVoice || _curContext->contextType()!=CAContext::Staff) {
			return false;
		}

		// lookup an element with the same type at the same time
		QList<CAMusElement*> foundElts = static_cast<CAStaff*>(_curContext)->getEltByType(CAMusElement::Barline, _curBarline->timeStart());
		CAMusElement *sign=0;
		for (int i=0; i<foundElts.size(); i++) {
			if (!foundElts[i]->compare(_curBarline))	  // element has exactly the same properties
				if (!_curVoice->contains(foundElts[i]))	{ // if such an element already exists, it means there are two different with the same timestart
					sign = foundElts[i];
					break;
				}
		}

		if (!sign) {
			// the element doesn't exist yet - add it to all the voices
			_curVoice->append( _curBarline );
		} else {
			// the element was found, insert only a reference to the current voice
			_curVoice->append( sign );
			delete _curBarline; _curBarline = 0;
		}
	} else if (qName == "note") {
		// CANote
		if ( _version.startsWith("0.5") ) {
		} else {
			_curNote->setPlayableLength( _curPlayableLength );
			if ( !_curNote->tuplet() ) {
				_curNote->calculateTimeLength();
			}
			_curNote->setDiatonicPitch( _curDiatonicPitch );
		}

		if (_curVoice->lastNote() && _curVoice->lastNote()->timeStart()==_curNote->timeStart())
			_curVoice->append( _curNote, true );
		else
			_curVoice->append( _curNote, false );

		_curNote->updateTies();
		_curNote = 0;
	} else if (qName == "tie") {
		// CASlur - tie
	} else if ( qName == "tuplet" ) {
		_curTuplet->assignTimes();
		_curTuplet = 0;
	} else if (qName == "rest") {
		// CARest
		if ( _version.startsWith("0.5") ) {
		} else {
			_curRest->setPlayableLength( _curPlayableLength );
			if ( !_curRest->tuplet() ) {
				_curRest->calculateTimeLength();
			}
		}

		_curVoice->append( _curRest );
		_curRest = 0;
	} else if (qName == "mark") {
		if ( !_version.startsWith("0.5") && _curMark->markType()==CAMark::Tempo ) {
			static_cast<CATempo*>(_curMark)->setBeat( _curPlayableLength );
		}
	} else if (qName == "function-mark") {
		if ( !_version.startsWith("0.5") && _curMusElt->musElementType()==CAMusElement::FunctionMark ) {
			static_cast<CAFunctionMark*>(_curMusElt)->setKey( _curDiatonicKey );
		}
	} else if (qName == "diatonic-key" ) {
		_curDiatonicKey.setDiatonicPitch( _curDiatonicPitch );
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
bool CACanorusMLImport::characters( const QString& ch ) {
	_cha = ch;

	return true;
}

void CACanorusMLImport::importMark( const QXmlAttributes& attributes ) {
	CAMark::CAMarkType type = CAMark::markTypeFromString(attributes.value("mark-type"));
	_curMark = 0;

	switch (type) {
	case CAMark::Text: {
		_curMark = new CAText(
			attributes.value("text"),
			static_cast<CAPlayable*>(_curMusElt)
		);
		break;
	}
	case CAMark::Tempo: {
		if ( _version.startsWith("0.5") ) {
			_curMark = new CATempo(
					CAPlayableLength( CAPlayableLength::musicLengthFromString(attributes.value("beat")), attributes.value("beat-dotted").toInt() ),
					attributes.value("bpm").toInt(),
					_curMusElt
			);
		} else {
			_curMark = new CATempo(
					CAPlayableLength(),
					attributes.value("bpm").toInt(),
					_curMusElt
			);
		}
		break;
	}
	case CAMark::Ritardando: {
		_curMark = new CARitardando(
			attributes.value("final-tempo").toInt(),
			static_cast<CAPlayable*>(_curMusElt),
			attributes.value("time-length").toInt(),
			CARitardando::ritardandoTypeFromString(attributes.value("ritardando-type"))
		);
		break;
	}
	case CAMark::Dynamic: {
		_curMark = new CADynamic(
			attributes.value("text"),
			attributes.value("volume").toInt(),
			static_cast<CANote*>(_curMusElt)
		);
		break;
	}
	case CAMark::Crescendo: {
		_curMark = new CACrescendo(
			attributes.value("final-volume").toInt(),
			static_cast<CANote*>(_curMusElt),
			CACrescendo::crescendoTypeFromString(attributes.value("crescendo-type")),
			attributes.value("time-start").toInt(),
			attributes.value("time-length").toInt()
		);
		break;
	}
	case CAMark::Pedal: {
		_curMark = new CAMark(
			CAMark::Pedal,
			_curMusElt,
			attributes.value("time-start").toInt(),
			attributes.value("time-length").toInt()
		);
		break;
	}
	case CAMark::InstrumentChange: {
		_curMark = new CAInstrumentChange(
			attributes.value("instrument").toInt(),
			static_cast<CANote*>(_curMusElt)
		);
		break;
	}
	case CAMark::BookMark: {
		_curMark = new CABookMark(
			attributes.value("text"),
			_curMusElt
		);
		break;
	}
	case CAMark::RehersalMark: {
		_curMark = new CAMark(
			CAMark::RehersalMark,
			_curMusElt
		);
		break;
	}
	case CAMark::Fermata: {
		if (_curMusElt->isPlayable()) {
			_curMark = new CAFermata(
				static_cast<CAPlayable*>(_curMusElt),
				CAFermata::fermataTypeFromString(attributes.value("fermata-type"))
			);
		} else if (_curMusElt->musElementType()==CAMusElement::Barline) {
			_curMark = new CAFermata(
				static_cast<CABarline*>(_curMusElt),
				CAFermata::fermataTypeFromString(attributes.value("fermata-type"))
			);
		}
		break;
	}
	case CAMark::RepeatMark: {
		_curMark = new CARepeatMark(
			static_cast<CABarline*>(_curMusElt),
			CARepeatMark::repeatMarkTypeFromString(attributes.value("repeat-mark-type")),
			attributes.value("volta-number").toInt()
		);
		break;
	}
	case CAMark::Articulation: {
		_curMark = new CAArticulation(
			CAArticulation::articulationTypeFromString(attributes.value("articulation-type")),
			static_cast<CANote*>(_curMusElt)
		);
		break;
	}
	case CAMark::Fingering: {
		QList<CAFingering::CAFingerNumber> fingers;
		for (int i=0; !attributes.value(QString("finger%1").arg(i)).isEmpty(); i++)
			fingers << CAFingering::fingerNumberFromString( attributes.value(QString("finger%1").arg(i)) );

		_curMark = new CAFingering(
			fingers,
			static_cast<CANote*>(_curMusElt),
			attributes.value("original").toInt()
		);
		break;
	}
	}

	if (_curMark)
		_curMusElt->addMark(_curMark);
}

/*!
	\fn CACanorusMLImport::document()
	Returns the newly created document when reading the XML file.
*/

/*!
	\var CACanorusMLImport::_cha
	Current characters being read using characters() method between the greater/lesser
	separators in XML file.

	\sa characters()
*/

/*!
	\var CACanorusMLImport::_depth
	Stack which represents the current depth of the document while SAX parsing. It contains
	the tag names as the values.

	\sa startElement(), endElement()
*/

/*!
	\var CACanorusMLImport::_errorMsg
	The error message content stored as QString, if the error happens.

	\sa fatalError()
*/

/*!
	\var CACanorusMLImport::_version
	Document program version - which Canorus saved the file?

	\sa startElement(), endElement()
*/

/*!
	\var CACanorusMLImport::_document
	Pointer to the document being read.

	\sa CADocument
*/
