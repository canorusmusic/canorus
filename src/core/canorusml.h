/** @file canorusml.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Georg Rudolph, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QIODevice>

#include "core/document.h"

#ifndef CANORUSML_H_
#define CANORUSML_H_

class CACanorusML {
	public:
		CACanorusML();
		~CACanorusML();
		
		static void saveDocument(QIODevice& out, CADocument *doc);
		static void openDocument(QIODevice& in, CADocument *doc);	
};

#endif /*CANORUSML_H_*/
