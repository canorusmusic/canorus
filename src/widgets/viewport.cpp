/** @file widgets/viewport.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "widgets/viewport.h"
#include "core/canorus.h"

CAViewPort::CAViewPort(QWidget *p) : QWidget(p) {
	_parent = p;
}

CAViewPort::~CAViewPort() {
	CACanorus::removeViewPort(this);
}
