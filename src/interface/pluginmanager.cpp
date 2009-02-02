/** @file interface/pluginmanager.cpp
 *
 * Copyright (c) 2006, 2007 Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 *
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "canorus.h"
#include "interface/pluginmanager.h"
#include "interface/plugin.h"
#include "interface/pluginaction.h"

#include <QDir>
#include <QFile>
#include <QXmlInputSource>
#include <QMenu>

// define static members
QList<CAPlugin*> CAPluginManager::_pluginList;
QMultiHash<QString, CAPlugin* > CAPluginManager::_actionMap;
QHash<QString, CAPluginAction* > CAPluginManager::_exportFilterMap;
QHash<QString, CAPluginAction* > CAPluginManager::_importFilterMap;

/*!
	\class CAPluginManager

	This class is the backend for loading, unloading, installing, removing and executing plugins.
	It's consists of two parts:
		- Static members
			Used globally to read, install/remove and enable/disable plugins.
		- Non-static members
			Inherits the QXml facilities and is used for parsing the plugin's descriptor file.

	A plugin is installed or uninstalled using installPlugin() or removePlugin() methods. installMethod()
	unpacks the compressed plugin into default user's plugins directory. removePlugin() deletes the plugin
	directory from the disk.

	After plugins are installed readPlugins() method should be called. This creates a list of available plugin
	objects and stores their location paths.

	To parse plugins descriptor files and enable plugins, call enablePlugins() to enable plugins marked as
	auto-load in Canorus config file or enablePlugin() to load a specific plugin. These methods use Qt's XML
	facilities to parse plugins descriptor files and use the non-static part of the class. They also require
	an already created main window as the parser creates menu structures, toolbars and other elements
	the plugin might offer.

 	An action (eg. when a user moves mouse in score viewport) is triggered by calling action() method and
 	pass the action type (eg. "onMouseMove") and other parameters. actionExport() and actionImport() are
 	separated and are called when Canorus user wants to export/import a document.

 	\todo Frontend window for plugin manipulation.

 	\sa CAPlugin, CAPluginManagerWin
*/

/*!
	Used if parsing plugin's descriptor file.
	It uses the given \a mainWin in order to create new menus and toolbars the plugin might include.
	If the plugin has already been created (eg. using the same plugin in multiple main windows)
	\a plugin is the existing plugin.
*/
CAPluginManager::CAPluginManager(CAMainWin *mainWin, CAPlugin *plugin) {
	_mainWin = mainWin;
	_curPlugin=plugin;
	_curPluginCanorusVersion = CANORUS_VERSION; /// \todo This should be read from the descriptor file as well
}

CAPluginManager::~CAPluginManager() {
}

/*!
	Reads the system and user's plugins directories and adds all the plugins to the internal plugins list.
	\warning This function doesn't enable or initialize any of the plugins - use enablePlugins() for this.

	\todo Add support for a user plugins directory.

	\sa enablePlugin(), enablePlugins()
*/
void CAPluginManager::readPlugins() {
	QString systemPluginsPath = QDir::searchPaths("plugins")[0];
	QList<QString> pluginPaths;

	// search the plugins paths and creates a list of directories for each plugin
	QDir curDir(systemPluginsPath);
	for (int j=0; j<curDir.count(); j++) {
		pluginPaths << curDir.absolutePath() + "/" + curDir[j];
	}

	for (int i=0; i<pluginPaths.size(); i++) {
		QXmlSimpleReader reader;
		QFile *file = new QFile(pluginPaths[i] + "/canorusplugin.xml");
		file->open(QIODevice::ReadOnly);

		// test if the descriptor file can be opened
		if (!file->isOpen()) {
			delete file;
			continue;
		}

		delete file;
		CAPlugin *plugin = new CAPlugin();
		plugin->setDirName(pluginPaths[i]);
		_pluginList << plugin;
	}
}

/*!
	Enables and initializes all plugins, which are marked as auto-load in Canorus config file.
	Returns true if all the plugins were successfully loaded, otherwise False.
	\todo auto-load not yet implemented. It currently enables all the plugins.

	\sa enablePlugin(), disablePlugins()
*/
bool CAPluginManager::enablePlugins(CAMainWin *mainWin) {
	bool res = true;
	for (int i=0; i<_pluginList.size(); i++) {
		if (!enablePlugin(_pluginList[i], mainWin))
			res=false;
	}

	return res;
}

/*!
	Disable and deinitializes all plugins.

	Return True, if all the plugins were successfully loaded, otherwise False.

	\sa disablePlugin(), enablePlugins()
*/
bool CAPluginManager::disablePlugins() {
	bool res = true;
	for (int i=0; i<_pluginList.size(); i++) {
		if (!disablePlugin(_pluginList[i]))
			res=false;
	}

	return res;
}

/*!
	Enables the plugin \a plugin and initializes it (action "onInit").

	Returns True, if the plugin was loaded successfully, otherwise False.

	\sa disablePlugin()
*/
bool CAPluginManager::enablePlugin(CAPlugin *plugin, CAMainWin *mainWin) {
	CAPluginManager *pm;
	QFile *file = new QFile(plugin->dirName() + "/canorusplugin.xml");
	file->open(QIODevice::ReadOnly);
	QXmlInputSource in(file);

	QXmlSimpleReader reader;
	reader.setContentHandler(pm = new CAPluginManager(mainWin, plugin));
	reader.parse(in);
	delete file;
	delete pm;

	if (plugin->isEnabled())
		// plugin was enabled before
		return true;

	// plugin wasn't enabled before, add its actions to local list
	QList<QString> actions = plugin->actionList();
	for (int i=0; i<actions.size(); i++) {
		_actionMap.insertMulti(actions[i], plugin);
	}

	plugin->setEnabled(true);
	return plugin->action("onInit", mainWin);
}

/*!
	Deinitializes the given \a plugin and remove any menus, toolbars and other GUI elements the plugin might
	have created from all the main windows.
	Plugin is unloaded, but still remains on the list - it's only disabled.

	Returns True, if plugin was unloaded successfully, otherwise False.
*/
bool CAPluginManager::disablePlugin(CAPlugin *plugin) {
	if (!plugin->isEnabled())
		return true;

	bool res = true;
	for (int i=0; i<CACanorus::mainWinCount(); i++)
		if (!plugin->action("onExit", CACanorus::mainWinAt(i)))
			res = false;

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
		for (int j=0; j<plugList.size(); j++)	// restore the hash - add deleted non-disabled actions of the other plugins back to the hash
			_actionMap.insertMulti(actions[i], plugList[j]);
	}

	return res;
}

/*!
	Extracts the plugin package at \a path to user's plugins directory.

	Returns True, if plugin was installed and loaded successfully, otherwise False.

	\sa removePlugin()
*/
bool CAPluginManager::installPlugin(QString path) {
	/// \todo zlib needed
}

/*!
	Disables and deletes the directory containing the given \a plugin.

	\sa installPlugin(), disablePlugin()
*/
bool CAPluginManager::removePlugin(CAPlugin *plugin) {
	disablePlugin(plugin);
	bool res = QFile::remove(plugin->dirName());

	delete plugin;
	return res;
}

bool CAPluginManager::startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& attributes) {
	_tree.push(qName);

	if (qName == "plugin") {
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
		if (qName == "separator") {
			_curPlugin->menu(_curActionParentMenu)->addSeparator();
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

/*!
	\fn bool CACanorus::exportFilterExists(const QString filter)

	Returns True if a plugin with the given filter in export dialog exists.
	Returns False if a plugin with such a filter doesn't exist. Internal LilyPond export code is usually
	used instead.

	\sa importFilterExists(const QString filter)
*/

/*!
	Finds the appropriate action having the given export \a filter and calls it using the given \a document and
	\a fileName.

	\sa importAction()
 */
void CAPluginManager::exportAction(QString filter, CADocument *document, QString filename) {
	_exportFilterMap[filter]->plugin()->callAction(_exportFilterMap[filter], 0, document, 0, 0, filename);
}

/*!
	\fn bool CACanorus::importFilterExists(const QString filter)

	Returns True if a plugin with the given filter in import dialog exists.
	Returns False if a plugin with such a filter doesn't exist. Internal LilyPond import code is usually
	used instead.

	\sa exportFilterExists(const QString filter)
*/

/*!
	Finds the appropriate action having the given import \a filter and calls it using the given \a document and
	\a fileName. The given \a document should already be created before calling this method.

	\sa exportAction()
*/
void CAPluginManager::importAction(QString filter, CADocument *document, QString filename) {
	_importFilterMap[filter]->plugin()->callAction(_importFilterMap[filter], 0, document, 0, 0, filename);
}

/*!
	Gathers all the plugins actions having the given \a val <action> <name> tag in its descriptor file and
	calls them.
	This method is usually triggered automatically by Canorus signals (like mouseClick on score viewport or
	a menu action).

	\sa importAction(), exportAction()
*/
void CAPluginManager::action(QString val, CADocument *document, QEvent *evt, QPoint *coords, CAMainWin *mainWin) {
	QList<CAPlugin*> list = _actionMap.values(val);

	for (int i=0; i<list.size(); i++) {
		list[i]->action(val, mainWin, document, evt, coords);
	}
}

/*!
	\var CACanorus::_pluginList

	List of both enabled and disabled plugins installed.
*/

/*!
	\var CACanorus::_actionMap

	Map of all the plugins actions names and actual plugins.
	This is used for faster (constant time) plugin look-up when running a specific action.

	\sa _exportActionMap, _importActionMap
*/

/*!
	\var CACanorus::_exportActionMap

	Map of all the plugins export actions names and actual plugins.
	This is used for faster (constant time) plugin look-up when running a specific action.

	\sa _actionMap, _importActionMap
*/

/*!
	\var CACanorus::_importActionMap

	Map of all the plugins import actions names and actual plugins.
	This is used for faster (constant time) plugin look-up when running a specific action.

	\sa _actionMap, _exportActionMap
*/

/*!
	\var CACanorus::_depth

	Hierarchy track of the current node.
*/
