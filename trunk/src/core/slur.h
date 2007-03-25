/* 
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#ifndef SLUR_H_
#define SLUR_H_

#include "core/muselement.h"

class CANote;

class CASlur : public CAMusElement {
public:
	enum CASlurDirection {
		SlurUp,
		SlurDown
	};
	
	enum CASlurStyle {
		SlurSolid,
		SlurDotted
	};
	
	enum CASlurType {
		Tie,
		Slur,
		PhrasingSlur
	};
	
	CASlur( CASlurType, CASlurDirection, CAContext *c, CANote *noteStart, CANote *noteEnd=0 );
	virtual ~CASlur();
	
	CASlur *clone();
	int compare( CAMusElement *elt );
	
	inline CASlurDirection slurDirection() { return _slurDirection; }
	inline void setSlurDirection(CASlurDirection dir) { _slurDirection = dir; }
	
	inline CASlurType slurType() { return _slurType; }
	inline CANote *noteStart() { return _noteStart; }
	inline CANote *noteEnd() { return _noteEnd; }
	inline CASlurStyle slurStyle() { return _slurStyle; }
	inline void setNoteStart( CANote *noteStart ) { _noteStart = noteStart; }
	inline void setNoteEnd( CANote *noteEnd ) { _noteEnd = noteEnd; }
	inline void setSlurStyle( CASlurStyle slurStyle ) { _slurStyle = slurStyle; }
	
private:
	inline void setSlurType( CASlurType type ) { _slurType = type; }
	
	CASlurDirection _slurDirection;
	CASlurStyle _slurStyle;
	CASlurType _slurType;
	
	CANote *_noteStart;
	CANote *_noteEnd;
};

#endif /*SLUR_H_*/
