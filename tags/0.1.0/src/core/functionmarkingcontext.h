/** @file core/functionmarkingcontext.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
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
		
		void addFunctionMarking(CAFunctionMarking *marking);
		QList<CAFunctionMarking*> *functionMarkingList() { return &_functionMarkingList; }
		
		int countFunctionMarkings(int timeStart);
		void clear();
		CAMusElement *findNextMusElement(CAMusElement *elt);
		CAMusElement *findPrevMusElement(CAMusElement *elt);
		bool removeMusElement(CAMusElement *elt, bool cleanup = true);
	
	private:
		QList<CAFunctionMarking*> _functionMarkingList;				///List of all the function markings sorted by timeStart
		QMultiHash<int,CAFunctionMarking*> _functionMarkingHash;	///Map of all the function markings in certain time slice - used by containsNewFunctionMarking() 
};

#endif /*FUNCTIONMARKINGCONTEXT_H_*/
