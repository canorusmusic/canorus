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
class CAStaff;
class CAClef;

class CAVoice {
	public:
		CAVoice(CAStaff *staff);
		CAStaff *staff() { return _staff; }
		void clear();
		
		void addClef(CAClef *clef);
	
	private:
		QList<CAMusElement *> _musElementList;
		CAStaff *_staff;	///Staff which this voice belongs to by default.
		int _voiceNumber;	///Number of the voice.
};

#endif /*VOICE_H_*/
