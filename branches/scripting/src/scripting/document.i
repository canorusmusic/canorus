/** @file document.i
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

%{
#include "core/document.h"
%}

/**
 * Swig implementation of CADocument.
 */
%rename(Document) CADocument;
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
		
		CASheet *sheet(const char* name);	///Look up for the sheet with the given name
		CASheet *sheetAt(int i);
		
		CASheet *addSheet(const char* name);
		void addSheet(CASheet *sheet);
                
                // rename all methods which return QString to qMethodName and all methods which return char* from c_strMethodName to methodName:
		%rename(qTitle) title;
                const QString title();
                %rename(title) c_strTitle;
                const char* c_strTitle();

                %rename(qSubTitle) subTitle;
		const QString subTitle();
                %rename(subTitle) c_strSubTitle;
		const char* c_strSubTitle();
                
                %rename(qComposer) composer;
		const QString composer();
                %rename(composer) c_strComposer;
		const char* c_strComposer();
                
                %rename(qArranger) arranger;
		const QString arranger();
                %rename(arranger) c_strArranger;
                const char* c_strArranger();
                
                %rename(qPoet) poet;
		const QString poet();
                %rename(poet) c_strPoet;
                const char* c_strPoet();
                
                %rename(qTextTranslator) textTranslator;
		const QString textTranslator();
                %rename(textTranslator) c_strTextTranslator;
                const char* c_strTextTranslator();
                
                %rename(qDedication) dedication;
		const QString dedication();
                %rename(dedication) c_strDedication;
                const char* c_strDedication();
                
                %rename(qCopyright) copyright;
		const QString copyright();
                %rename(copyright) c_strCopyright;
                const char* c_strCopyright();

                %rename(qTimestamp) timestamp;
		const QString timestamp();
                %rename(timestamp) c_strTimestamp;
                const char* c_strTimestamp();
                
                %rename(qComments) comments;
		const QString comments();
                %rename(comments) c_strComments;
                const char* c_strComments();
		
		void setTitle(const char *title);
		void setSubTitle(const char* subTitle);
		void setComposer(const char* composer);
		void setArranger(const char* arranger);
		void setPoet(const char* poet);
		void setTextTranslator(const char* textTranslator);
		void setDedication(const char* dedication);
		void setCopyright(const char* copyright);
		void setTimestamp(const char* timestamp);
		void setComments(const char* comments);
};

