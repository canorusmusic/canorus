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

class CAVoice;

class CACanorusML : public QXmlDefaultHandler {
	public:
		CACanorusML();
		~CACanorusML();
		
		static void saveDocument(QTextStream& out, CADocument *doc);
		static void openDocument(QTextStream& in, CADocument *doc);
	
	private:
		static const QString createMLVoice(CAVoice *v);
		
		inline static const QString idn(int depth) {	///Append the number of tabs to the string
			QString ret;
			for (int i=0; i<depth; i++)
				ret += "\t";
			return ret;
		} 
};

#endif /*CANORUSML_H_*/
