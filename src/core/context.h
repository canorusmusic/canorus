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

#include "drawable/drawable.h"

class CASheet;


class CAContext {
	public:
		CAContext(CASheet *s);
		
		/**
		 * enum CAContextType holds different CAContext types:
		 * - Staff - Every context with lines which includes various music elements.
		 * - Tablature - Context similar to Staff, but specialized for guitar.
		 * - Lyrics - Holds words (syllables) for choir music. Every syllable is assigned to certain CANote.
		 * - Dynamics - Sometimes we want to have a separate context for the crescendo, rit., mf and other markings
		 */
		enum CAContextType {
			Staff,
			Tablature,
			Lyrics,
			Dynamics
		};

		/**
		 * Set the Context's name.
		 * 
		 * @param name Context's name in QString format
		 */
		void setName(const QString name) { _name = name; }
		virtual void clear() = 0;
		CAContextType contextType() { return _contextType; }
		
	protected:
		CASheet *_sheet;
		QString _name;
		CAContextType _contextType;
};

#endif /*CONTEXT_H_*/
