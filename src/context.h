/** @file context.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef CONTEXT_H_
#define CONTEXT_H_

#include "drawable.h"

class CASheet;

class CAContext : public CADrawable {
	public:
		CAContext(CASheet *s, int x, int y);
		
		virtual void clear() = 0;
		
	protected:
		CASheet *_sheet;
};

#endif /*CONTEXT_H_*/
