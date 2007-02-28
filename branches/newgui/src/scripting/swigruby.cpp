/** @file scripting/swigruby.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifdef USE_RUBY
#include "scripting/swigruby.h"
#include <QFile>
#include <QDir>

//defined in SWIG wrapper class
extern "C" void Init_CanorusRuby();	///Load 'CanorusRuby' module and initialize classes
QString locateResource(QString);
QString locateResourceDirectory(QString);

void CASwigRuby::init() {
	ruby_init();
	Init_CanorusRuby();
	// add path to scripts to Scripting path
	rb_eval_string((QString("$: << '") + locateResource("scripts") + "'").toStdString().c_str());
	// add path to CanorusRuby module to Scripting path
#ifdef Q_WS_WIN
	rb_eval_string((QString("$: << '") + locateResourceDirectory("CanorusRuby.dll") + "'").toStdString().c_str());
#else
	rb_eval_string((QString("$: << '") + locateResourceDirectory("CanorusRuby.so") + "'").toStdString().c_str());
#endif
}

VALUE CASwigRuby::callFunction(QString fileName, QString function, QList<VALUE> args) {
	if (!QFile::exists(fileName))
		return 0;
	
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
