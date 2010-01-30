/*!
	Copyright (c) 2006-2010, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef LAYOUTENGINE_
#define LAYOUTENGINE_

#include <QList>
#include <QMap>

class CAScoreView;
class CADrawableMusElement;
class CAContext;
class CADrawableContext;

class CALayoutEngine {
public:
	CALayoutEngine( CAScoreView *v );
	void reposit();

private:
	void placeMarks( CADrawableMusElement*, int );
	void updateContextsWidth();

	CAScoreView *_scoreView;
	int *_streamsRehersalMarks;
	QList<CADrawableMusElement*> _scalableElts;
	QMap<CAContext*, CADrawableContext*> _drawableContextMap;

	static const double INITIAL_X_OFFSET; // space between the left border and the first music element
	static const double RIGHT_X_OFFSET;   // space after the last element in the staff
	static const double MINIMUM_SPACE;    // minimum space between the music elements
};

#endif /* LAYOUTENGINE_ */
