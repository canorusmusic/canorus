/** @file canorusml.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Georg Rudolph, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
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

/**
 * Class used for writing/opening a file.
 * For writing or opening the document, simply use the static methods saveDocument() and openDocument().
 * The methods themselves create a non-static instance of the object used for parsing the XML then.
 */
class CACanorusML : public QXmlDefaultHandler {
	public:
		~CACanorusML();
		
		////////////////////////////////////////////////
		//Reading XML
		////////////////////////////////////////////////
		static CADocument* openDocument(QXmlInputSource* in, CAMainWin *mainWin);
		
		/**
		 * Default constructor.
		 * 
		 * @param doc Pointer to the document where the elements should be added to.
		 */
		CACanorusML(CAMainWin *mainWin);
		bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName,
		                  const QXmlAttributes& attributes);
        bool endElement(const QString& namespaceURI, const QString& localName,
		                const QString& qName);
		bool fatalError(const QXmlParseException& exception);
		bool characters(const QString& ch);
		
		/**
		 * Return the document being created when reading the xml file.
		 * 
		 * @return Pointer to the newly created document
		 */
		CADocument *document() { return _document; }
		
		////////////////////////////////////////////////
		//Writing XML
		////////////////////////////////////////////////
		static void saveDocument(QTextStream& out, CADocument *doc);
		
	private:
		////////////////////////////////////////////////
		//Reading XML
		////////////////////////////////////////////////
		/** OBSOLETE */
		bool readMusElements(QString string);	///Read music elements between the greater/lesser separators and add them to the document.
		
		CADocument     *_document;	/// Pointer to the document being read
		CAMainWin      *_mainWin;	/// Pointer to the main window of the application - needed to rebuild the UI
		QString         _version;	/// Document version
		QString         _errorMsg;	/// The error message content, if the error happens
		QStack<QString> _depth;	/// Stack which represents the current depth of the document. It contains the tag names as the values.
		
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
			//Temporary properties for each XML stanza
			////////////////////////////////////////////
			QString _cha;	///Current characters being read using characters() method between the greater/lesser separators.
		
		////////////////////////////////////////////////
		//Writing XML
		////////////////////////////////////////////////
		static void writeVoice(QDomElement &dVoice, CAVoice* voice);
		
		/** DEPRECATED */
		static const QString createMLVoice(CAVoice *v);
		
		/** DEPRECATED */
		inline static const QString idn(int depth) {	///Append the number of tabs to the string
			QString ret;
			for (int i=0; i<depth; i++)
				ret += "\t";
			return ret;
		} 
};

#endif /*CANORUSML_H_*/
