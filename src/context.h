/** @file context.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef CONTEXT_H_
#define CONTEXT_H_

#include <QString>

#include "drawable.h"

class CASheet;

class CAContext {
	public:
		enum {CAStaff, CATablature, CALyrics, CADynamics};

		CAContext(CASheet *s);
		
		/**
		 * Set the Context's name.
		 * 
		 * @param name Context's name in QString format
		 */
		void setName(const QString name) { _name = name; }
		virtual void clear() = 0;
		char type() { return _type; }
		
	protected:
		CASheet *_sheet;
		QString _name;
		char _type;
};

#endif /*CONTEXT_H_*/
