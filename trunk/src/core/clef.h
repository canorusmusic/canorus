/** @file clef.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef CLEF_H_
#define CLEF_H_

#include <QString>

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
		CAClef *clone();
		
		/**
		 * This is an overloaded member function provided for convenience.
		 * 
		 * Constructor using the ML arguments.
		 */ 
		CAClef(const QString type, CAStaff *staff, int time);
		
		CAClefType clefType() { return _clefType; }
		const QString clefTypeML();
		const int c1() { return _c1; }
		const int centerPitch() { return _centerPitch; }
		int compare(CAMusElement *elt);
		
	private:
		CAClefType _clefType;
		int _c1;	///Location of c'. 0 = 1st line, 1 = 1st space, -2 = 1st ledger line below staff
		int _centerPitch;	///Location of the clef's beginning - f for bas clef, g' for treble clef, c for c-clefs etc.
};

#endif /*CLEF_H_*/
