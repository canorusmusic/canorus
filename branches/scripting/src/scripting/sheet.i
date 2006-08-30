/** @file sheet.i
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

%module Ca

%{
#include "core/sheet.h"
%}

/**
 * Swig implementation of CASheet.
 */
class CASheet {
	public:
		CASheet(const QString name, CADocument *doc);
		
		CAContext *contextAt(int i);
		CAContext *context(const QString name);	///Look up for the context with the given name.
		void addContext(CAContext *);
		inline int contextCount();
		
		CAStaff *addStaff();
		inline int staffCount();
		CAStaff *staffAt(int i);

		CADocument *document();
		const QString name();
		void setName(const QString name);

		void clear();
};
