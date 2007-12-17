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
		Volta,
		Segno,
		Coda,
		VarCoda
	};
	
	CARepeatMark( CABarline *b, CARepeatMarkType t );
	virtual ~CARepeatMark();
	
	inline CARepeatMarkType repeatMarkType() { return _repeatMarkType; }
	inline void setRepeatMarkType( CARepeatMarkType t ) { _repeatMarkType = t; }
	
private:
	CARepeatMarkType _repeatMarkType;
};

#endif /* REPEATMARK_H_ */
