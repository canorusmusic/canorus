/* 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

%{
#include "core/slur.h"
%}

/**
 * Swig implementation of CASlur.
 */
%rename(Slur) CASlur;
class CASlur : public CAMusElement {
public:
	enum CASlurDirection {
		SlurUp,
		SlurDown
	};
	
	CASlur( CASlurDirection, CAContext *c, CANote *noteStart, CANote *noteEnd=0 );
	virtual ~CASlur();
	
	inline CASlurDirection slurDirection() { return _slurDirection; }
	inline void setSlurDirection(CASlurDirection dir) { _slurDirection = dir; }
	
	inline CANote *noteStart() { return _noteStart; }
	inline CANote *noteEnd() { return _noteEnd; }
	inline void setNoteStart( CANote *noteStart ) { _noteStart = noteStart; }
	inline void setNoteEnd( CANote *noteEnd ) { _noteEnd = noteEnd; }
};
