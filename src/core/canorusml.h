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
#include "core/voice.h"

class CAVoice;
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
		//Writing XML
		////////////////////////////////////////////////
		static void saveDocument(QTextStream& out, CADocument *doc);
		
		////////////////////////////////////////////////
		//Reading XML
		////////////////////////////////////////////////
		/**
		 * Default constructor.
		 * 
		 * @param doc Pointer to the document where the elements should be added to.
		 */
		CACanorusML(CADocument *doc, CAMainWin *mainWin);
		static void openDocument(QIODevice* in, CADocument *doc, CAMainWin *mainWin);
		bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName,
		                  const QXmlAttributes& attributes);
        bool endElement(const QString& namespaceURI, const QString& localName,
		                const QString& qName);
		bool fatalError(const QXmlParseException& exception);
		bool characters(const QString& ch);
		
	private:
		////////////////////////////////////////////////
		//Reading XML
		////////////////////////////////////////////////
		CADocument *_document;
		CAMainWin *_mainWin;	///Pointer to the main window of the application - needed to rebuild the UI
		QString _errorMsg;	///The error message content, if the error happens
		QStack<QString> _depth;	///Stack which represents the current depth of the document. It contains the tag names as the values.
		void readMusElements(const QString string);	///Read music elements between the greater/lesser separators and add them to the document.
		
		CASheet *_curSheet;
		CAContext *_curContext;
		CAVoice *_curVoice;	///Pointer to the current voice when reading the musElements.
		
			////////////////////////////////////////////
			//Temporary properties for each XML stanza
			////////////////////////////////////////////
			QString _cha;	///Current characters being read using characters() method between the greater/lesser separators.
			QString _diatonicGender;	///CAKeySignature::CADiatonicGenderType

		////////////////////////////////////////////////
		//Writing XML
		////////////////////////////////////////////////
		static const QString createMLVoice(CAVoice *v);
		
		inline static const QString idn(int depth) {	///Append the number of tabs to the string
			QString ret;
			for (int i=0; i<depth; i++)
				ret += "\t";
			return ret;
		} 
};

#endif /*CANORUSML_H_*/
