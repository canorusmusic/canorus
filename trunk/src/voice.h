/** @file voice.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef VOICE_H_
#define VOICE_H_

#include <QList>

class CAMusElement;

class CAVoice {
	public:
		void clear();
	
	private:
		QList<CAMusElement *> _musElementList;
};

#endif /*VOICE_H_*/
