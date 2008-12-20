/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef MUSICXMLIMPORT_H_
#define MUSICXMLIMPORT_H_

#include <QXmlDefaultHandler>
#include <QString>
#include <QStack>
#include <QHash>
#include <QMultiHash>

#include "core/playablelength.h"
#include "core/diatonicpitch.h"
#include "import/import.h"

class CADocument;
class QDomNode;
class CASheet;
class CAStaff;
class CAKeySignature;
class CATimeSignature;

class CAMusicXmlImport: public CAImport, public QXmlDefaultHandler {
public:
	enum CAMusicXmlType {
		TimeWise,
		PartWise,
		Undefined
	};

	CAMusicXmlImport( QTextStream *stream=0 );
	CAMusicXmlImport( const QString stream );
	virtual ~CAMusicXmlImport();

	bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& attributes);
	bool endElement(const QString& namespaceURI, const QString& localName, const QString& qName);
	bool fatalError(const QXmlParseException& exception);
	bool characters(const QString& ch);

private:
	void initMusicXmlImport();
	CADocument* importDocumentImpl();

	bool parseDocumentStartElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& attributes);
	bool parseDocumentEndElement(const QString& namespaceURI, const QString& localName, const QString& qName);
	bool parsePartListStartElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& attributes);
	bool parsePartListEndElement(const QString& namespaceURI, const QString& localName, const QString& qName);
	bool parsePartStartElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& attributes);
	bool parsePartEndElement(const QString& namespaceURI, const QString& localName, const QString& qName);

	CAMusicXmlType  _type;
	QString         _version;
	QStack<QString> _depth;
	QString         _cha;
	QString         _errorMsg;

	CADocument *_document;
	CASheet    *_curSheet;
	CAStaff    *_curStaff;
	QHash<QString, CAStaff*> _staffMap;
	QHash<CAStaff*, int> _midiChannel;
	QHash<CAStaff*, int> _midiProgram;
	QMultiHash<CAStaff*, CAVoice* > _voiceMap;

	// Part attributes
	QMultiHash<CAStaff*, int> _divisions;
	QMultiHash<CAStaff*, CAKeySignature*> _keySignature;
	QMultiHash<CAStaff*, CATimeSignature*> _timeSignature;


	// Notes and rests
	CAPlayableLength::CAMusicLength _playableLength;
	bool _playableRest;
	int _playableVoiceNumber;
	CADiatonicPitch _notePitch;



	QString _creatorType;
};

#endif /* MUSICXMLIMPORT_H_ */
