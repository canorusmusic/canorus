/** @file interface/engraver.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef ENGRAVER_
#define ENGRAVER_

class CAScoreViewPort;

/**
 * This class is a bridge between the data part of Canorus and the UI.
 * Out of data CAMusElement* and CAContext* objects, it creates their CADrawable* instances.
 */
class CAEngraver {
	public:
		static void reposit(CAScoreViewPort *v);
};

#endif /*ENGRAVER_*/
