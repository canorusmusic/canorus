/** @file document.i
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

%module Ca

%{
#include "core/document.h"
%}

/**
 * Swig implementation of CADocument.
 */
class CADocument {
	public:
		CADocument();
		void clear();
		
		/**
		 * Return the number of created sheets in the document.
		 * 
		 * @return Number of sheets in the document.
		 */
		int sheetCount();
		
		CASheet *sheet(const QString name);	///Look up for the sheet with the given name
		CASheet *sheetAt(int i);
		
		CASheet *addSheet(const QString name);
		void addSheet(CASheet *sheet);

		const QString title();
		const QString subTitle();
		const QString composer();
		const QString arranger();
		const QString poet();
		const QString textTranslator();
		const QString dedication();
		const QString copyright();
		const QString timestamp();
		const QString comments();
		
		void setTitle(const QString title);
		void setSubTitle(const QString subTitle);
		void setComposer(const QString composer);
		void setArranger(const QString arranger);
		void setPoet(const QString poet);
		void setTextTranslator(const QString textTranslator);
		void setDedication(const QString dedication);
		void setCopyright(const QString copyright);
		void setTimestamp(const QString timestamp);
		void setComments(const QString comments);
};
