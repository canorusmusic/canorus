/** @file staff.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef STAFF_H_
#define STAFF_H_

#include <QList>

#include "context.h"
#include "voice.h"

class CAStaff : public CAContext {
	public:
		void clear();
		
	private:
		QList<CAVoice *> _voiceList;
};

#endif /*STAFF_H_*/
