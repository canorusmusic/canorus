/** @file ui/pluginaction.cpp
 * 
 * Copyright (c) 2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "ui/pluginaction.h"
#include "ui/mainwin.h"

CAPluginAction::CAPluginAction(CAPlugin *plugin, QString name, QString lang, QString function, QList<QString> args, QString filename)
 : QAction(0) {
	_plugin = plugin;
	_name = name;
	_lang = lang;
	_function = function;
	_filename = filename;
	_args = args;
	
	connect(this, SIGNAL(triggered(bool)), this, SLOT(triggeredSlot(bool)));
}

void CAPluginAction::triggeredSlot(bool on) {
	QObject *curObject = this;
	while (dynamic_cast<CAMainWin*>(curObject)==0 && curObject!=0) // find the parent which is mainwindow
		curObject = curObject->parent();
	
	_plugin->callAction(this, (CAMainWin*)curObject, ((CAMainWin*)curObject)->curDocument(), 0, 0);
}
