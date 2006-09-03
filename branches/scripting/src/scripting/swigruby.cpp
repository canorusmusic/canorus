#include "scripting/swigruby.h"
#include <iostream>

VALUE CASwigRuby::callFunction(QString module, QString function, QList<VALUE> args) {
	ruby_init();
	init();
	
	QString path = "/home/matevz/canorus/branches/scripting/src/";
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
	QString path = "/home/matevz/canorus/branches/scripting/src/";
	//standard CanorusRuby packet
	QString requireString = path + "CanorusRuby";
	rb_require(requireString.toStdString().c_str());
}
