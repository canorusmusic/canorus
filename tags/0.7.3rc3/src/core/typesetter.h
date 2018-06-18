/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef TYPESETTER_H_
#define TYPESETTER_H_

class CATypesetter {
public:
	enum CATypesetterType { // used for storing the default typesetter in settings
		LilyPond = 1
	};

	CATypesetter();
	virtual ~CATypesetter();
};

#endif /* TYPESETTER_H_ */
