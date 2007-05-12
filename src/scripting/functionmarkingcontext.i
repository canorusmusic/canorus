/** @file scripting/functionmarkingcontext.i
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

%{
#include "core/functionmarkingcontext.h"
%}

/**
 * Swig implementation of CAFunctionMarking.
 */
%rename(FunctionMarkingContext) CAFunctionMarkingContext;
class CAFunctionMarkingContext : public CAContext {
	public:
		CAFunctionMarkingContext(CASheet *sheet, const QString name);
		~CAFunctionMarkingContext();
		
		int functionMarkingCount(int timeStart);
		void addFunctionMarking(CAFunctionMarking *marking);
		QList<CAFunctionMarking*> functionMarkingList() { return _functionMarkingList; }		
		
		void clear();
		CAMusElement *findNextMusElement(CAMusElement *elt);
		CAMusElement *findPrevMusElement(CAMusElement *elt);
		bool removeMusElement(CAMusElement *elt, bool cleanup);
};