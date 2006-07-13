/** @file clef.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef CLEF_H_
#define CLEF_H_

#include "muselement.h"

class CAStaff;

class CAClef : public CAMusElement {
	public:
		enum CAClefType {
			Treble,
			Bass,
			Alto,
			Tenor,
			Soprano,
			PercussionHight,
			PercussionLow
		};

		CAClef(CAClefType type, CAStaff *staff, int time);
			
		CAClefType clefType() { return _clefType; }
	
	private:
		CAClefType _clefType;
};

#endif /*CLEF_H_*/
