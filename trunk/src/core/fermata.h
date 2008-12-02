/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef FERMATA_H_
#define FERMATA_H_

#include "core/mark.h"

class CAPlayable;
class CABarline;

class CAFermata : public CAMark {
public:
	enum CAFermataType {
		NormalFermata,
		ShortFermata,
		LongFermata,
		VeryLongFermata
	};
	
	CAFermata( CAPlayable *m, CAFermataType t = NormalFermata );
	CAFermata( CABarline *b,  CAFermataType t = NormalFermata );
	virtual ~CAFermata();

	CAFermata *clone(CAMusElement* elt=0);
	int compare( CAMusElement* );
	
	inline CAFermataType fermataType() { return _fermataType; }
	inline void setFermataType( CAFermataType t ) { _fermataType = t; }
	
	static const QString fermataTypeToString( CAFermataType t );
	static CAFermataType fermataTypeFromString( const QString r );
	
private:
	CAFermataType _fermataType;
};

#endif /* FERMATA_H_ */
