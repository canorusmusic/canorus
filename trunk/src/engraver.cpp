/** @file engraver.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "engraver.h"
#include "scoreviewport.h"
#include "sheet.h"
#include "drawablestaff.h"

void CAEngraver::reposit(CAScoreViewPort *v) {
	int i;
	CASheet *sheet = v->sheet();
	
	int dy = 100;
	for (int i=0; i < sheet->contextList()->size(); i++, dy+=200) {
		v->addCElement(new CADrawableStaff( ((CAStaff*)(sheet->contextList()->at(i))), 0, dy));
	}
}
