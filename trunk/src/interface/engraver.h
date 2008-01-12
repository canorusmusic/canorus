/*!
	Copyright (c) 2006, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef ENGRAVER_
#define ENGRAVER_

#include <QList>

class CAScoreViewPort;
class CADrawableMusElement;

class CAEngraver {
	public:
		static void reposit(CAScoreViewPort *v);
	private:
		static void placeMarks( CADrawableMusElement*, CAScoreViewPort*, int );
		static int *streamsRehersalMarks;
		static QList<CADrawableMusElement*> scalableElts;
};

#endif /* ENGRAVER_ */
