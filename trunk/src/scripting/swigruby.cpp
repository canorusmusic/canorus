/** @file scripting/swigruby.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "scripting/swigruby.h"
#include <QCoreApplication>	//needed to determine the scripts path

//defined in canorusruby.i
extern "C" void Init_CanorusRuby();	///Loads 'CanorusRuby' module and initializes classes

VALUE CASwigRuby::callFunction(QString module, QString function, QList<VALUE> args) {
	QString path = QCoreApplication::applicationDirPath() + "/scripts/";	//this should be done on more dynamic way (probably different path when Canorus is installed?)
	//require module (loads a method)
	QString requireString = path + module;
	rb_require(requireString.toStdString().c_str());
	
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
