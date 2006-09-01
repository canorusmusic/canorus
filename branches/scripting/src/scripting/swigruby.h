/** @file swigruby.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <ruby.h>

#include <QString>
#include <QList>

class CASwigRuby {
	public:
		static VALUE callFunction(QString module, QString function, QList<VALUE> args);
};

