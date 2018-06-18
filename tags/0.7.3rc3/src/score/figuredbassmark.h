/*!
	Copyright (c) 2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef FIGUREDBASSMARK_H_
#define FIGUREDBASSMARK_H_

#include "score/muselement.h"
#include <QHash>
#include <QList>

class CAFiguredBassContext;

class CAFiguredBassMark: public CAMusElement {
public:
	CAFiguredBassMark( CAFiguredBassContext *c, int timeStart, int timeLength );
	~CAFiguredBassMark();

	CAMusElement* clone(CAContext* context=0);
	int compare(CAMusElement *elt);

	void addNumber( int number );
	void addNumber( int number, int accs );
	void removeNumber( int number );

	QList<int>&      numbers() { return _numbers; }
	QHash<int, int>& accs() { return _accs; }

private:
	void insertNumber( int number );

	QList<int>      _numbers; // sorted numbers in the figured bass mark
	QHash<int, int> _accs;
};

#endif /* FIGUREDBASSMARK_H_ */
