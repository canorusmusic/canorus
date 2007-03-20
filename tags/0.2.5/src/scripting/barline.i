/** @file scripting/barline.i
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

%{
#include "core/barline.h"
%}

/**
 * Swig implementation of CABarline.
 */
%rename(Barline) CABarline;
class CABarline : public CAMusElement{
	public:
		enum CABarlineType {
			Undefined=-1,
			Single,
			Double,
			End,
			RepeatOpen,
			RepeatClose,
			RepeatCloseOpen,
			Dotted
		};
		
		CABarline(CABarlineType type, CAStaff *staff, int startTime);
		
		~CABarline();
		
		CABarlineType barlineType();
		CABarline *clone();
		int compare(CAMusElement *elt);
};
