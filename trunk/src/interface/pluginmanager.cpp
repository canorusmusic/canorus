/** @file interface/pluginmanager.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "interface/pluginmanager.h"
#include "interface/plugin.h"
#include <iostream>
#include <QDir>
#include <QFile>
#include <QXmlInputSource>

CAPluginManager::CAPluginManager(CAMainWin *mainWin) {
	_mainWin = mainWin;
	_curPlugin=0;
}

CAPluginManager::~CAPluginManager() {
}

void CAPluginManager::readPlugins() {
	QDir pluginsDir("src/plugins");
	
	for (int i=0; i<pluginsDir.count(); i++) {
		QXmlSimpleReader reader;
		QFile *file = new QFile(pluginsDir.absolutePath() + "/" + pluginsDir[i] + "/canorusplugin.xml");
		file->open(QIODevice::ReadOnly);
		if (!file->isOpen())
			continue;
		
		QXmlInputSource in(file);
		reader.setContentHandler(this);
		reader.parse(in);
		_curPlugin = 0;
	}
}

bool CAPluginManager::enablePlugins() {
	bool res = true;
	for (int i=0; i<_pluginList.size(); i++) {
		if (!enablePlugin(_pluginList[i]))
			res=false;
	}
	
	return res;
}

bool CAPluginManager::enablePlugin(CAPlugin *plugin) {
	if (plugin->enabled())
		return false;
	
	QList<QString> actions = plugin->actionList();
	for (int i=0; i<actions.size(); i++) {
		_actionMap.insertMulti(actions[i], plugin);
	}
	
	plugin->setEnabled(true);
	return plugin->action("onInit", _mainWin);
}

bool CAPluginManager::disablePlugin(CAPlugin *plugin) {
	if (!plugin->enabled())
		return false;
	
	plugin->setEnabled(false);
	bool res = plugin->action("onExit", _mainWin);

	QList<QString> actions = plugin->actionList();
	for (int i=0; i<actions.size(); i++) {	//QMultiHash doesn't support remove(key, value), we have to do this manually
		QList<CAPlugin*> plugList;
		CAPlugin *val;
		while ((val = _actionMap.take(actions[i])) != plugin) {
			plugList << val;
		}
		for (int j=0; j<plugList.size(); i++)
			_actionMap.insertMulti(actions[i], plugList[j]);
	}
	
	return res;
}

void CAPluginManager::action(QString val, CADocument *document, QEvent *evt, QPoint *coords) {
	QList<CAPlugin*> list = _actionMap.values(val);
	
	for (int i=0; i<list.size(); i++) {
		list[i]->action(val, _mainWin, document, evt, coords);
	}
}

bool CAPluginManager::installPlugin(QString path) {
	//TODO - zlib needed
}

bool CAPluginManager::removePlugin(CAPlugin *plugin) {
	disablePlugin(plugin);
	bool res = QFile::remove(plugin->dirName());
	
	delete plugin;
	return res;
}

bool CAPluginManager::startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& attributes) {
	if (qName=="plugin") {
		_curPlugin = new CAPlugin();
		_pluginList << _curPlugin;
	}
	
	if (_curPlugin) {
		if (qName=="name") {
			_curLang = attributes.value("lang");
		} else
		if (qName=="action") {
			QString onAction = attributes.value("onAction");
			QString lang = attributes.value("lang");
			QString fileName = attributes.value("fileName");
			QString function = attributes.value("function");
			
			//read the function arguments arg1, arg2, arg3 etc. until argN is empty
			QList<QString> args;
			for (int i=1; attributes.value(QString("arg")+QString::number(i))!=""; i++)
				args << attributes.value(QString("arg")+QString::number(i));
			
			_curPlugin->addAction(onAction, lang, fileName, function, args);
		}
	}
	
	return true;
}

bool CAPluginManager::endElement(const QString& namespaceURI, const QString& localName, const QString& qName) {
	if (_curPlugin) {
		if (qName == "name") {
			_curPlugin->setName(_curChars, _curLang);
		} else
		if (qName == "version") {
			_curPlugin->setVersion(_curChars);
			if (!_curPlugin->name().isEmpty()) {
				_curPlugin->setDirName(QDir::current().absolutePath() + "/src/plugins/" + _curPlugin->name().toLower() + "-" + _curPlugin->version());
			}
		} else
		if (qName == "homeUrl") {
			_curPlugin->setHomeUrl(_curChars);
		} else
		if (qName == "updateUrl") {
			_curPlugin->setUpdateUrl(_curChars);
		}
	}
	
	return true;
}

bool CAPluginManager::fatalError(const QXmlParseException& exception) {
	return false;
}

bool CAPluginManager::characters(const QString& ch) {
	_curChars = ch;
	return true;
}
