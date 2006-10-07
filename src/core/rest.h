/** @file core/rest.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
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
		
		CARest(CAPlayableLength length, CAVoice *voice, int timeStart, CARestType type, int timeLength=0);
		CARest *clone();
		
		~CARest();

		const QString lengthML();
		const QString restTypeML();
		
		CARestType restType() { return _restType; }
		
		int compare(CAMusElement *elt);
	
	private:
		CARestType _restType;
};

#endif /*REST_H_*/
