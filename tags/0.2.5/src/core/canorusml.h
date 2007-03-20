/*
 * Copyright (c) 2006-2007, Matevž Jekovec, Georg Rudolph, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 *
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#ifndef CANORUSML_H_
#define CANORUSML_H_

#include <QTextStream>
#include <QtXml>

#include "core/document.h"

class QXmlInputSource;

class CAVoice;
class CAKeySignature;
class CATimeSignature;
class CABarline;
class CAClef;
class CANote;
class CARest;
class CAContext;
class CAMainWin;

class CACanorusML : public QXmlDefaultHandler {
public:
	~CACanorusML();
	
	////////////////////////////////////////////////
	// Reading XML
	////////////////////////////////////////////////
	CACanorusML(CAMainWin *mainWin);
	static CADocument* openDocument(QXmlInputSource* in, CAMainWin *mainWin);
	
	bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName,
	                  const QXmlAttributes& attributes);
	bool endElement(const QString& namespaceURI, const QString& localName,
	                const QString& qName);
	bool fatalError(const QXmlParseException& exception);
	bool characters(const QString& ch);
	
	inline CADocument *document() { return _document; }
	
	////////////////////////////////////////////////
	// Writing XML
	////////////////////////////////////////////////
	static void saveDocument(CADocument *doc, QTextStream& out);
	
private:
	////////////////////////////////////////////////
	// Reading XML
	////////////////////////////////////////////////
	
	CADocument     *_document;
	CAMainWin      *_mainWin;
	QString         _version;
	QString         _errorMsg;
	QStack<QString> _depth;
	
	// Pointers to the current elements when reading the XML file
	CASheet         *_curSheet;
	CAContext       *_curContext;
	CAVoice         *_curVoice;
	CAKeySignature  *_curKeySig;
	CATimeSignature *_curTimeSig;
	CAClef          *_curClef;
	CABarline       *_curBarline;
	CANote          *_curNote;
	CARest          *_curRest;
	
		////////////////////////////////////////////
		// Temporary properties for each XML stanza
		////////////////////////////////////////////
		QString _cha;
	
	////////////////////////////////////////////////
	// Writing XML
	////////////////////////////////////////////////
	static void writeVoice(QDomElement &dVoice, CAVoice* voice);
};
#endif /* CANORUSML_H_*/
