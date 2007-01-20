/** @file interface/pluginmanager.cpp
 * 
 * Copyright (c) 2006, 2007 Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "interface/pluginmanager.h"
#include "interface/plugin.h"
#include "ui/mainwin.h"	//needed for locateResource()
#include "ui/pluginaction.h"
#include <iostream>
#include <QDir>
#include <QFile>
#include <QXmlInputSource>
#include <QMenu>

CAPluginManager::CAPluginManager(CAMainWin *mainWin) {
	_mainWin = mainWin;
	_curPlugin=0;
	_curPluginCanorusVersion = CANORUS_VERSION;
}

CAPluginManager::~CAPluginManager() {
	disablePlugins();
	for (int i=0; i<_pluginList.size(); i++) {
		delete _pluginList[i];
	}
}

void CAPluginManager::readPlugins() {
	QDir pluginsDir(locateResource("plugins"));
	
	for (int i=0; i<pluginsDir.count(); i++) {
		QXmlSimpleReader reader;
		QFile *file = new QFile(pluginsDir.absolutePath() + "/" + pluginsDir[i] + "/canorusplugin.xml");
		file->open(QIODevice::ReadOnly);
		if (!file->isOpen())
		{
			delete file;
			continue;
		}
		
		QXmlInputSource in(file);
		reader.setContentHandler(this);
		reader.parse(in);
		_curPlugin->setDirName(pluginsDir.absolutePath() + "/" + pluginsDir[i]);	//use absolute path again because we want slashes/backslashes to be unified
		_curPlugin = 0;
		delete file;
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

bool CAPluginManager::disablePlugins() {
	bool res = true;
	for (int i=0; i<_pluginList.size(); i++) {
		if (!disablePlugin(_pluginList[i]))
			res=false;
	}
	
	return res;
}

bool CAPluginManager::enablePlugin(CAPlugin *plugin) {
	if (plugin->isEnabled())
		return true;
	
	QList<QString> actions = plugin->actionList();
	for (int i=0; i<actions.size(); i++) {
		_actionMap.insertMulti(actions[i], plugin);
	}
	
	plugin->setEnabled(true);
	return plugin->action("onInit", _mainWin);
}

bool CAPluginManager::disablePlugin(CAPlugin *plugin) {
	if (!plugin->isEnabled())
		return true;
	
	bool res = plugin->action("onExit", _mainWin);
	plugin->setEnabled(false);
	
	// remove plugin specific actions from generic plugins actions list
	QList<QString> actions = plugin->actionList();
	for (int i=0; i<actions.size(); i++) {	// QMultiHash doesn't support remove(key, value) or remove(value), only remove(key) - we have to do this manually now
		QList<CAPlugin*> plugList;
		while (CAPlugin *val = _actionMap.take(actions[i])) {
			if (val != plugin) {	// while val exists and != plugin
				plugList << val;	// remember deleted values which don't belong to the disabled plugin
			}
		}
		for (int j=0; j<plugList.size(); i++)	// restore the hash - add deleted non-disabled actions of the other plugins back to the hash
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
	_tree.push(qName);
	
	if (qName == "plugin") {
		_curPlugin = new CAPlugin();
		_pluginList << _curPlugin;
	}
	
	if (_curPlugin) {
		if (qName == "description") {
			_curPluginLocale = attributes.value("lang");
		} else
		if (qName == "text") {
			_curActionLocale = attributes.value("lang");
		} else
		if (qName == "title") {
			_curMenuLocale = attributes.value("lang");
		} else
		if (qName == "export-filter") {
		} else
		if (qName == "import-filter") {
		} else
		if (qName == "action") {
			_curActionLang = "python"; _curActionFunction.clear(); _curActionFilename.clear();
			_curActionArgs.clear();
			_curActionText.clear();
			_curActionLocale.clear();
			_curActionName.clear();
			_curActionExportFilter.clear();
			_curActionImportFilter.clear();
			_curActionOnAction.clear();
			_curActionParentMenu.clear();
			_curActionParentToolbar.clear();
			_curActionRefresh = false;
		} else
		if (qName == "menu") {
			_curMenuTitle.clear();
			_curMenuName.clear();
			_curMenuLocale.clear();
			_curMenuParentMenu.clear();
		} else
		if (qName == "toolbar") {
		}
	}
	
	return true;
}

bool CAPluginManager::endElement(const QString& namespaceURI, const QString& localName, const QString& qName) {
	_tree.pop();
	
	if (_curPlugin) {
		// top-level tags
		if (qName == "canorus-version") {
			_curPluginCanorusVersion = _curChars;
		} else
		if (qName == "name") {
			if (_tree.back()=="plugin")
				_curPlugin->setName(_curChars);
			else if (_tree.back()=="action")
				_curActionName = _curChars;
			else if (_tree.back()=="menu")
				_curMenuName = _curChars;
		} else
		if (qName == "version") {
			_curPlugin->setVersion(_curChars);
		} else
		if (qName == "author") {
			_curPlugin->setAuthor(_curChars);
		} else
		if (qName == "home-url") {
			_curPlugin->setHomeUrl(_curChars);
		} else
		if (qName == "update-url") {
			_curPlugin->setUpdateUrl(_curChars);
		} else
		if (qName == "description") {
			_curPlugin->setDescription(_curChars, _curPluginLocale);
		} else
		if (qName == "action") {
			CAPluginAction *action = new CAPluginAction(_curPlugin, _curActionName, _curActionLang, _curActionFunction, _curActionArgs, _curActionFilename);
			if (!_curActionParentMenu.isEmpty()) {
				action->setParent(_mainWin);
				_curPlugin->menu(_curActionParentMenu)->addAction(action);
			}
			
			action->setOnAction(_curActionOnAction);
			action->setExportFilters(_curActionExportFilter);
			action->setImportFilters(_curActionImportFilter);
			action->setTexts(_curActionText);
			action->setRefresh(_curActionRefresh);
			
			if (!_curActionParentToolbar.isEmpty());
				// TODO: add action to toolbar
			
			// Add import and export filters to the generic list for faster lookup
			QList<QString> filters;
			filters = _curActionExportFilter.values();
			for (int i=0; i<filters.size(); i++) {
				_exportFilterMap[filters[i]] = action;
				_mainWin->exportDialog()->setFilters(_mainWin->exportDialog()->filters() << filters[i]);
			}
			
			filters = _curActionImportFilter.values();
			for (int i=0; i<filters.size(); i++) {
				_importFilterMap[filters[i]] = action;
				_mainWin->importDialog()->setFilters(_mainWin->importDialog()->filters() << filters[i]);
			}
			
			_curPlugin->addAction(action);
		} else
		if (qName == "menu") {
			QMenu *menu;
			if (_curMenuParentMenu.isEmpty()) {
				// no parent menu set, add it to the top-level mainwindow's menu before the Help menu
				menu = new QMenu(_mainWin->menuBar());
				_mainWin->menuBar()->insertMenu(_mainWin->menuBar()->actions().last(), menu);
			} else {
				// parent menu set, find it and add a new submenu to it
				menu = new QMenu(_curPlugin->menu(_curMenuParentMenu));
				_curPlugin->menu(_curMenuParentMenu)->addMenu(menu);
			}
			menu->setObjectName(_curMenuName);
			
			if (_curMenuTitle.contains(QLocale::system().name()))
				menu->setTitle( _curMenuTitle[QLocale::system().name() ]);
			else
				menu->setTitle( _curMenuTitle[""] );
			
			_curPlugin->addMenu(_curMenuName, menu);
		} else
		// action level
		if (qName == "on-action") {
			_curActionOnAction = _curChars;
		} else
		if (qName == "lang") {
			if (_tree.back()=="action")
				_curActionLocale = _curChars;
			else if (_tree.back()=="menu")
				_curMenuLocale = _curChars;				
		} else
		if (qName == "function") {
			_curActionFunction = _curChars;
		} else
		if (qName == "filename") {
			_curActionFilename = _curChars;
		} else
		if (qName == "text") {
			_curActionText[_curActionLocale] = _curChars;
		} else
		if (qName == "args") {
			_curActionArgs << _curChars;
		} else
		if (qName == "parent-menu") {
			_curActionParentMenu = _curChars;
		} else
		if (qName == "export-filter") {
			_curActionExportFilter[_curActionLocale] = _curChars;
		} else
		if (qName == "import-filter") {
			_curActionImportFilter[_curActionLocale] = _curChars;
		} else
		if (qName == "refresh") {
			_curActionRefresh = true;
		} else
		// menu level
		if (qName == "title") {
			_curMenuTitle[_curMenuLocale] = _curChars;
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

void CAPluginManager::exportAction(QString filter, CADocument *document, QEvent *evt, QPoint *coords, QString filename) {
	_exportFilterMap[filter]->plugin()->callAction(_exportFilterMap[filter], _mainWin, document, evt, coords, filename);
}

void CAPluginManager::importAction(QString filter, CADocument *document, QEvent *evt, QPoint *coords, QString filename) {
	_importFilterMap[filter]->plugin()->callAction(_importFilterMap[filter], _mainWin, document, evt, coords, filename);
}
