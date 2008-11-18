/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef REPEATMARK_H_
#define REPEATMARK_H_

#include "core/mark.h"

class CABarline;

class CARepeatMark : public CAMark {
public:
	enum CARepeatMarkType {
		Undefined  = 0,
		Volta      = 1,
		Segno      = 2,
		Coda       = 3,
		VarCoda    = 4,
		DalSegno   = 5,
		DalCoda    = 6,
		DalVarCoda = 7
	};
	
	CARepeatMark( CABarline *b, CARepeatMarkType t, int voltaNumber = 0 );
	virtual ~CARepeatMark();
	
	CARepeatMark *clone(CAMusElement* elt=0);
	int compare( CAMusElement *);
	
	inline CARepeatMarkType repeatMarkType() { return _repeatMarkType; }
	inline void setRepeatMarkType( CARepeatMarkType t ) { _repeatMarkType = t; }
	
	inline int voltaNumber() { return _voltaNumber; }
	inline void setVoltaNumber( int n ) { _voltaNumber = n; }
	
	static const QString repeatMarkTypeToString( CARepeatMarkType t );
	static CARepeatMarkType repeatMarkTypeFromString( const QString r );
	
private:
	CARepeatMarkType _repeatMarkType;
	int _voltaNumber;
};

#endif /* REPEATMARK_H_ */
