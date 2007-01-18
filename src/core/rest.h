/*
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 *
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#ifndef REST_H_
#define REST_H_

#include "core/playable.h"

class CAStaff;

class CARest : public CAPlayable {
public:
	enum CARestType {
		Normal,
		Hidden
	};
	
	CARest(CARestType type, CAPlayableLength length, CAVoice *voice, int timeStart, int dotted=0);
	CARest *clone();
	
	~CARest();
	
	const QString lengthML();   // Deprecated
	const QString restTypeML(); // Deprecated
	
	CARestType restType() { return _restType; }
	
	int compare(CAMusElement *elt);
	
	static const QString restTypeToString(CARestType);
	static CARestType restTypeFromString(const QString);
	
private:
	CARestType _restType;
};
#endif /*REST_H_*/
