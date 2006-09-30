/** @file scripting/rest.i
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

%{
#include "core/rest.h"
%}

/**
 * Swig implementation of CARest.
 */
%rename(Rest) CARest;
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
};
