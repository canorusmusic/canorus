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
			PercussionHigh,
			PercussionLow
		};

		CAClef(CAClefType type, CAStaff *staff, int time);
		
		CAClefType clefType() { return _clefType; }
		const QString clefTypeML();
		const int c1() { return _c1; } 
	
	private:
		CAClefType _clefType;
		int _c1;	///Location of c'. 0 = 1st line, 1 = 1st space, -2 = 1st ledger line below staff
};

#endif /*CLEF_H_*/
