/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "ui/pluginaction.h"
#include "ui/mainwin.h"

/*!
	\class CAPluginAction
	CAPluginAction class represents each <action> stanza found in Canorus Plugin descriptor XML file.
	Every action has its unique name, scripting language, function, its arguments and filename.
	Optional properties are Canorus internal action which it reacts on, export filter, menu/toolbar text.
*/

/*!
	Default constructor.
*/
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

/*!
	This method and the class itself exists mainly because you can't connect signal actionMyAction_triggered()
	and others than connecting them with slots. And to do that, you need a Q_OBJECT class with pre-set
	function slots. This function is a pretty elegant solution to connect plugin's reactions to internal
	Canorus GUI signals.
*/
void CAPluginAction::triggeredSlot(bool on) {
	QObject *curObject = this;
	while (dynamic_cast<CAMainWin*>(curObject)==0 && curObject!=0) // find the parent which is mainwindow
		curObject = curObject->parent();
	
	_plugin->callAction(this, (CAMainWin*)curObject, ((CAMainWin*)curObject)->document(), 0, 0);
}

