/** @file scripting/clef.i
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

%{
#include "core/clef.h"
%}

/**
 * Swig implementation of CAClef.
 */
%rename(Clef) CAClef;
class CAClef : public CAMusElement {
	public:
		enum CAClefType {
			Undefined=-1,
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
		
		CAClefType clefType();
		const int c1();
		const int centerPitch();
		int compare(CAMusElement *elt);
		
		void setClefType(CAClefType type);
		
};
