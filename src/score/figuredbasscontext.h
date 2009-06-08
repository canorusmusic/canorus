/*!
	Copyright (c) 2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef FIGUREDBASSCONTEXT_H_
#define FIGUREDBASSCONTEXT_H_

#include "score/context.h"
#include <QList>

class CAFiguredBassMark;

class CAFiguredBassContext: public CAContext {
public:
	CAFiguredBassContext( QString name, CASheet *sheet );
	~CAFiguredBassContext();

	CAContext* clone( CASheet* );
	void clear();
	CAMusElement *next(CAMusElement *elt);
	CAMusElement *previous(CAMusElement *elt);
	bool remove( CAMusElement *elt );

	QList<CAFiguredBassMark*>& figuredBassMarkList() { return _figuredBassMarkList; }
	CAFiguredBassMark *figuredBassMarkAtTimeStart( int timeStart );

	void repositFiguredBassMarks();
	void addFiguredBassMark( CAFiguredBassMark*, bool replace=true );
	void addEmptyFiguredBassMark( int timeStart, int timeLength );

private:
	QList<CAFiguredBassMark*> _figuredBassMarkList;
};

#endif /* FIGUREDBASSCONTEXT_H_ */
