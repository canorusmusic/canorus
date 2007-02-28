/*
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 *
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#ifndef CLEF_H_
#define CLEF_H_

#include <QString>

#include "core/muselement.h"

class CAStaff;

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
	
	CAClef(const QString type, CAStaff *staff, int time);
	
	CAClefType clefType() { return _clefType; }
	const int c1() { return _c1; }
	const int centerPitch() { return _centerPitch; }
	int compare(CAMusElement *elt);
	
	void setClefType(CAClefType type);
	
	static const QString clefTypeToString(CAClefType);
	static CAClefType clefTypeFromString(const QString);
	
private:
	CAClefType _clefType;
	int _c1; // Location of middle C in the staff
	int _centerPitch;	// Location of the clefs physical center
};
#endif /*CLEF_H_*/
