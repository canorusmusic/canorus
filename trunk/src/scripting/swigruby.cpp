/** @file scripting/swigruby.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifdef USE_RUBY
#include "scripting/swigruby.h"
#include <QDir>

//defined in SWIG wrapper class
extern "C" void Init_CanorusRuby();	///Load 'CanorusRuby' module and initialize classes

void CASwigRuby::init() {
	ruby_init();
	Init_CanorusRuby();
}

VALUE CASwigRuby::callFunction(QString fileName, QString function, QList<VALUE> args) {
	//require module (loads a method)
	rb_require(QDir::convertSeparators(fileName).toStdString().c_str());
	
	//call function
	VALUE recv = 0;
	VALUE argsArray[args.size()];
	for (int i=0; i<args.size(); i++)
		argsArray[i] = args[i];
	
	return rb_funcall2(recv, rb_intern(function.toStdString().c_str()), args.size(), argsArray);
}

#endif
