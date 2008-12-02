/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef CLEF_H_
#define CLEF_H_

#include <QString>

#include "core/muselement.h"

class CAStaff;
class CAContext;

class CAClef : public CAMusElement {
public:
	enum CAPredefinedClefType {
		Undefined=-1,
		Treble,
		Bass,
		French,
		Soprano,
		Mezzosoprano,
		Alto,
		Tenor,
		Baritone,
		Varbaritone,
		Subbass,
		Percussion,
		Tablature
	};
	
	enum CAClefType {
		F,
		G,
		C,
		PercussionHigh,
		PercussionLow,
		Tab
	};
	
	CAClef( CAPredefinedClefType type, CAStaff *staff, int time, int offsetInterval=0 );
	CAClef( CAClefType type, int c1, CAStaff *staff, int time, int offset=0 );
	CAClef *clone(CAContext* context=0);
	
	void setPredefinedType( CAPredefinedClefType type );
	CAClefType clefType() { return _clefType; }
	const int c1() { return _c1; }
	const int centerPitch() { return _centerPitch; }
	int compare(CAMusElement *elt);
	
	void setClefType(CAClefType type);
	
	inline void setOffset( int offset ) { _c1+=_offset; _c1-=(_offset=offset); }
	inline int offset() { return _offset; }
	
	static const QString clefTypeToString(CAClefType);
	static CAClefType clefTypeFromString(const QString);
	
	static const int offsetFromReadable( const int offsetInterval );
	static const int offsetToReadable( const int offset );
	
private:
	CAClefType _clefType;
	int _c1;          // Location of middle C in the staff
	int _centerPitch; // Location of the clefs physical center (where the clef's glyph is going to be rendered)
	int _offset;
};
#endif /* CLEF_H_ */
