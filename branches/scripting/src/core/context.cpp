/** @file context.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "context.h"

CAContext::CAContext(CASheet *s, const QString name) {
	_sheet = s;
	_name = name;
}
