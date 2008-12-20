/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QDomDocument>
#include <QDebug>
#include "import/musicxmlimport.h"

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

CAMusicXmlImport::CAMusicXmlImport( QTextStream *stream )
 : CAImport(stream), QXmlDefaultHandler() {
	initMusicXmlImport();
}

CAMusicXmlImport::CAMusicXmlImport( const QString stream )
 : CAImport(stream), QXmlDefaultHandler() {
	initMusicXmlImport();
}

CAMusicXmlImport::~CAMusicXmlImport() {
}

void CAMusicXmlImport::initMusicXmlImport() {
	_document = 0;
	_curSheet = 0;
	_type = Undefined;
}

/*!
	Opens a MusicXML source \a in and creates a document out of it.
	CAMusicXmlImport uses SAX model for reading.
*/
CADocument* CAMusicXmlImport::importDocumentImpl() {
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
	reader->setErrorHandler( this );
	reader->parse( src );

	delete reader;
	delete src;

	return _document;
}

bool CAMusicXmlImport::startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& attributes) {
	parseDocumentStartElement( namespaceURI, localName, qName, attributes );
	parsePartListStartElement( namespaceURI, localName, qName, attributes );
	parsePartStartElement( namespaceURI, localName, qName, attributes );

	_depth.push(localName);
	return true;
}

bool CAMusicXmlImport::endElement(const QString& namespaceURI, const QString& localName, const QString& qName) {
	parseDocumentEndElement( namespaceURI, localName, qName );
	parsePartListEndElement( namespaceURI, localName, qName );
	parsePartEndElement( namespaceURI, localName, qName );

	_depth.pop();
	_cha = "";

	return true;
}

bool CAMusicXmlImport::parseDocumentStartElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& attributes ) {
	if ( localName=="score-partwise" ) {
		_type = PartWise;
		_document = new CADocument();
	} else if ( localName=="score-timewise" ) {
		_type = TimeWise;
		_document = new CADocument();
	} else if ( localName=="creator" ) {
		_creatorType=attributes.value("type");
	}

	return true;
}

bool CAMusicXmlImport::parseDocumentEndElement(const QString& namespaceURI, const QString& localName, const QString& qName) {
	if (_document) {
		if ( localName=="work-title" ) {
			_document->setTitle( _cha );
		} else
		if ( localName=="creator") {
			if ( _creatorType=="composer" ) {
				_document->setComposer( _cha );
			} else
			if ( _creatorType=="lyricist" ) {
				_document->setPoet( _cha );
			} else
			if ( _creatorType=="rights" ) {
				_document->setCopyright( _cha );
			}
			_creatorType="";
		} else
		if (localName=="score-part") {
			_curStaff=0;
		} else
		if (localName=="part-list") {
			_curSheet=0;
		}
	}

	return true;
}

bool CAMusicXmlImport::parsePartListStartElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& attributes ) {
	if (_document && _curSheet) {
		if (localName=="part-list") {
			_curSheet = new CASheet( tr("Sheet%1").arg("1"), _document );
			_document->addSheet( _curSheet );
		} else
		if ( localName=="score-part" ) {
			CAStaff *staff = new CAStaff( "", _curSheet );
			_staffMap[ attributes.value("id" ) ] =  staff;
		}
	}

	return true;
}

bool CAMusicXmlImport::parsePartListEndElement( const QString& namespaceURI, const QString& localName, const QString& qName ) {
	if (_document && _curSheet) {
		if (localName=="part-name" && _curStaff) {
			_curStaff->setName( _cha );
		} else
		if (localName=="midi-channel" && _curStaff) {
			_midiChannel[_curStaff] = _cha.toInt();
		} else
		if (localName=="midi-program" && _curStaff) {
			_midiProgram[_curStaff] = _cha.toInt();
		}
	}

	return true;
}

bool CAMusicXmlImport::parsePartStartElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& attributes ) {
	if ( localName=="part" ) {
		_curStaff = _staffMap[attributes.value("id")];
	}

	return true;
}

bool CAMusicXmlImport::parsePartEndElement( const QString& namespaceURI, const QString& localName, const QString& qName ) {
	if ( localName=="part") {
		_curStaff = 0;
	} else
	if ( localName=="measure" && _curStaff ) {
		CABarline *barline = new CABarline( CABarline::Single, _curStaff, 0 );
		//for (int i=0; i<_voiceMap[_curStaff].size(); i++) {
		//	_voiceMap[_curStaff].at(i)->append( barline );
		//}
	}

	return true;
}

bool CAMusicXmlImport::fatalError(const QXmlParseException& exception) {
	qWarning() << "Fatal error on line " << exception.lineNumber()
		<< ", column " << exception.columnNumber() << ": "
		<< exception.message() << "\n\nParser message:\n" << _errorMsg;

	return false;
}

bool CAMusicXmlImport::characters(const QString& ch) {
	_cha = ch;

	return true;
}
