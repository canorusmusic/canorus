/* 
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */


#ifndef BARLINE_H_
#define BARLINE_H_

#include "core/muselement.h"

class CAStaff;

class CABarline : public CAMusElement{
public:
	enum CABarlineType {
		Undefined = -1,
		Single,
		Double,
		End,
		RepeatOpen,
		RepeatClose,
		RepeatCloseOpen,
		Dotted
	};
	
	CABarline(CABarlineType type, CAStaff *staff, int startTime);		
	virtual ~CABarline();
	
	CABarline *clone();
	int compare(CAMusElement* elt);
	
	CABarlineType barlineType() { return _barlineType; }		
	
	static const QString barlineTypeToString(CABarlineType);
	static CABarlineType barlineTypeFromString(const QString);
	
private:
	CABarlineType _barlineType;
};
#endif /*BARLINE_H_*/
