/*!
	Copyright (c) 2006-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef LAYOUTENGINE_
#define LAYOUTENGINE_

#include <QList>

class CAScoreView;
class CADrawableMusElement;

class CALayoutEngine {
	public:
		static void reposit(CAScoreView *v);
	private:
		static void placeMarks( CADrawableMusElement*, CAScoreView*, int );
		static int *streamsRehersalMarks;
		static QList<CADrawableMusElement*> scalableElts;
};

#endif /* LAYOUTENGINE_ */
