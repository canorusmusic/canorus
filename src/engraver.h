/** @file engraver.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef ENGRAVER_
#define ENGRAVER_

class CASheet;
class CAScrollWidget;

class CAEngraver {
	public:
		static void reposit(CASheet *s, CAScrollWidget *c);
};

#endif /*ENGRAVER_*/
