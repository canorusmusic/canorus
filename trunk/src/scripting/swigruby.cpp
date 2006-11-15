/** @file scripting/swigruby.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifdef USE_RUBY
#include "scripting/swigruby.h"
#include <QCoreApplication>	//needed to determine the scripts path

//defined in canorusruby.i
extern "C" void Init_CanorusRuby();	///Loads 'CanorusRuby' module and initializes classes

VALUE CASwigRuby::callFunction(QString fileName, QString function, QList<VALUE> args) {
	//require module (loads a method)
	rb_require(fileName.toStdString().c_str());
	
	//call function
	VALUE recv = 0;
	VALUE argsArray[args.size()];
	for (int i=0; i<args.size(); i++)
		argsArray[i] = args[i];
	
	return rb_funcall2(recv, rb_intern(function.toStdString().c_str()), args.size(), argsArray);
}

void CASwigRuby::init() {
	ruby_init();
	Init_CanorusRuby();
}

#endif
