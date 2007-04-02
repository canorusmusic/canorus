/* 
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#ifndef FUNCTIONMARKINGCONTEXT_H_
#define FUNCTIONMARKINGCONTEXT_H_

#include <QString>
#include <QList>
#include <QMultiHash>

#include "core/context.h"

class CASheet;
class CAFunctionMarking;

class CAFunctionMarkingContext : public CAContext {
public:
	CAFunctionMarkingContext(CASheet *sheet, const QString name);
	~CAFunctionMarkingContext();
	
	int functionMarkingCount(int timeStart=0);
	inline CAFunctionMarking *functionMarkingAt(int idx) { return _functionMarkingList[idx]; }
	void addFunctionMarking(CAFunctionMarking *marking);
	inline QList<CAFunctionMarking*> functionMarkingList() { return _functionMarkingList; }
		
	void clear();
	CAMusElement *findNextMusElement(CAMusElement *elt);
	CAMusElement *findPrevMusElement(CAMusElement *elt);
	bool removeMusElement(CAMusElement *elt, bool cleanup = true);
	
private:
	QList<CAFunctionMarking*> _functionMarkingList;
	QMultiHash<int,CAFunctionMarking*> _functionMarkingHash;
};
#endif /* FUNCTIONMARKINGCONTEXT_H_*/
