/*!
	Copyright (c) 2006, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifdef USE_RUBY
#ifndef SWIGCPP
#include "canorus.h"
#endif
#include "scripting/swigruby.h"
#include <QFile>
#include <QDir>

/// Load 'CanorusRuby' module and initialize classes - defined in SWIG wrapper class
extern "C" void Init_CanorusRuby();	

void CASwigRuby::init() {
	ruby_init();
	Init_CanorusRuby();
	// add path to scripts to Scripting path
	if (QDir::searchPaths("scripts").size())
		rb_eval_string((QString("$: << '") + QDir::searchPaths("scripts")[0] + "'").toStdString().c_str());
	// add path to CanorusRuby module to Scripting path
#ifdef Q_WS_WIN
	if (QFileInfo("base:CanorusRuby.dll").exists())
		rb_eval_string((QString("$: << '") + QFileInfo("base:CanorusRuby.dll").absolutePath() + "'").toStdString().c_str());
#else
	if (QFileInfo("base:CanorusRuby.so").exists())
		rb_eval_string((QString("$: << '") + QFileInfo("base:CanorusRuby.so").absolutePath() + "'").toStdString().c_str());
#endif
}

VALUE CASwigRuby::callFunction(QString fileName, QString function, QList<VALUE> args) {
	if (!QFile::exists(fileName))
		return 0;
	
	// require module (loads a method)
	rb_require(QDir::convertSeparators(fileName).toStdString().c_str());
	
	// call function
	VALUE recv = 0;
	VALUE argsArray[args.size()];
	for (int i=0; i<args.size(); i++)
		argsArray[i] = args[i];
	
	return rb_funcall2(recv, rb_intern(function.toStdString().c_str()), args.size(), argsArray);
}

#endif
