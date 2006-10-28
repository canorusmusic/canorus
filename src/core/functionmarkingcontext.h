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

#include "core/context.h"

class CASheet;
class CAFunctionMarking;

class CAFunctionMarkingContext : public CAContext {
	public:
		CAFunctionMarkingContext(CASheet *sheet, QString name);
		~CAFunctionMarkingContext();
		
		void addFunctionMarking(CAFunctionMarking *marking);
		
	
	private:
		QList<CAFunctionMarking*> _functionMarkingList;
};

#endif /*FUNCTIONMARKINGCONTEXT_H_*/
