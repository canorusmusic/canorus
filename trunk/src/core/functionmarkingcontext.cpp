/** @file core/functionmarkingcontext.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "core/functionmarkingcontext.h"
#include "core/functionmarking.h"

CAFunctionMarkingContext::CAFunctionMarkingContext(CASheet *sheet, QString name)
 : CAContext(sheet, name) {
 	_contextType = CAContext::FunctionMarkingContext;
}

CAFunctionMarkingContext::~CAFunctionMarkingContext() {
}
