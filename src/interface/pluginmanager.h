/** @file interface/pluginmanager.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef PLUGINMANAGER_H_
#define PLUGINMANAGER_H_

#include <QString>
#include <QMultiHash>
#include <QXmlDefaultHandler>

class CAMainWin;
class CADocument;
class CAPlugin;

class QEvent;
class QPoint;

/**
 * This class is used for loading, unloading, installing, removing and executing plugins.
 * A plugin is installed or uninstalled using installPlugin() or removePlugin() methods.
 * A plugin is loaded or unloaded using loadPlugin() or unloadPlugin() methods.
 * An action (eg. when a user moves mouse in score viewport) is triggered calling action() method and passing action descriptor (eg. "onMouseMove").
 */
class CAPluginManager : public QXmlDefaultHandler {
	public:
		CAPluginManager(CAMainWin *mainWin);
		~CAPluginManager();
		
		/**
		 * Read the plugins directory and add all the plugins to the plugins list.
		 * Warning: This function doesn't enable or initialize any of the plugins - use enablePlugins() for this.
		 */
		void readPlugins();
		
		/**
		 * Enable and initialize all plugins, which are marked as run-at-startup.
		 * 
		 * @return True, if all the plugins were successfully loaded, False otherwise.
		 */
		bool enablePlugins();
		
		/**
		 * Enable the plugin
		 * 
		 * @param plugin Pointer to plugin to be loaded.
		 * @return True, if plugin was loaded successfully, False otherwise.
		 */
		bool enablePlugin(CAPlugin* plugin);
		
		/**
		 * Disable a plugin and deinitialize it.
		 * Warning: Plugin is unloaded, but still remains on the list - it's only disabled.
		 * 
		 * @return True, if plugin was unloaded successfully, False otherwise.
		 */
		bool disablePlugin(CAPlugin* plugin);
		
		/**
		 * Call plugins actions.
		 * 
		 * @param val Name of the action.
		 */
		void action(QString val, CADocument *document, QEvent *evt, QPoint *coords);
		
		/**
		 * Decompress the .zip containing the plugin to plugins directory and load it.
		 * 
		 * @param path Path to .zip file containing the plugin.
		 * @return True, if plugin was installed and loaded successfully, False otherwise.
		 */
		bool installPlugin(QString path);
		
		/**
		 * Uninstall the specified plugin.
		 */
		bool removePlugin(CAPlugin* plugin);
		
		const QList<CAPlugin*> pluginList() { return _pluginList; };
		
		//XML parser functions:
		bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName,
		                  const QXmlAttributes& attributes);
        bool endElement(const QString& namespaceURI, const QString& localName,
		                const QString& qName);
		bool fatalError(const QXmlParseException& exception);
		bool characters(const QString& ch);
		
	private:
		QList<CAPlugin*> _pluginList;	///List of all the plugins installed
		QMultiHash<QString, CAPlugin*> _actionMap;	///List of all plugins loaded accessed by their actions for faster lookup
		CAMainWin *_mainWin;
		
		CAPlugin *_curPlugin;
		QString _curChars;
		QString _curLang;
};

#endif /*PLUGINMANAGER_H_*/
