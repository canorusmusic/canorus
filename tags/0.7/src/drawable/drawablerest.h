/*!
	Copyright (c) 2006-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef DRAWABLEREST_H_
#define DRAWABLEREST_H_

#include "drawable/drawablemuselement.h"
#include "core/rest.h"

class CADrawableRest : public CADrawableMusElement {
	public:
		CADrawableRest(CARest *rest, CADrawableContext *drawableContext, double x, double y);
		CADrawableRest *clone(CADrawableContext* newContext = 0);
		~CADrawableRest();

		void draw(QPainter *p, CADrawSettings s);

		inline CARest* rest() { return static_cast<CARest*>(_musElement); }

	private:
		double _restWidth;	///Width of the rest itself without dots, ledger lines etc.
};

#endif /*DRAWABLEREST_H_*/
