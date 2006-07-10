/** @file engraver.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "engraver.h"
#include "scrollwidget.h"
#include "sheet.h"

void CAEngraver::reposit(CASheet *s, CAScrollWidget *c) {
	int i;
	
	for (int i=0; i<s->contextList()->size(); i++) {
		c->addElement((CADrawable*)s->contextList()->at(i));
	}
}
