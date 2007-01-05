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
#include <QStack>

class CAMainWin;
class CADocument;
class CAPlugin;
class CAPluginAction;

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
		 * Disable and deinitialize all plugins.
		 * This method is called automatically on destructor.
		 * 
		 * @return True, if all the plugins were successfully loaded, False otherwise.
		 */
		bool disablePlugins();
		
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
		 * Gather all the plugins actions having the given onAction value and call them.
		 * This method is usually triggered automatically by Canorus signals (like mouseClick on score viewport).
		 * 
		 * @param onAction onAction name value.
		 */
		void action(QString onAction, CADocument *document, QEvent *evt, QPoint *coords);
		
		/**
		 * Find the appropriate action having the given export filter and call it.
		 * 
		 * @param filter Action's filter in export dialog.
		 * @param document Pointer to the current document.
		 * @param evt Pointer to the Mouse/Key/Wheel event, if it happened.
		 * @param coords Pointer to the coords in absolute world units, if a click on ScoreViewPort happened.
		 */
		void exportAction(QString filter, CADocument *document, QEvent *evt, QPoint *coords);
		bool exportActionExists(QString action) { return _exportFilterMap.contains(action); }
		
		/**
		 * Find the appropriate action having the given import filter and call it.
		 * 
		 * @param filter Action's filter in import dialog.
		 * @param document Pointer to the current document.
		 * @param evt Pointer to the Mouse/Key/Wheel event, if it happened.
		 * @param coords Pointer to the coords in absolute world units, if a click on ScoreViewPort happened.
		 */
		void importAction(QString filter, CADocument *document, QEvent *evt, QPoint *coords);
		bool importActionExists(QString action) { return _importFilterMap.contains(action); }
		
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
		QList<CAPlugin*> _pluginList;                     /// List of all the plugins installed
		QMultiHash<QString, CAPlugin* > _actionMap;        /// List of all plugins loaded accessable by their actions for faster lookup
		QHash<QString, CAPluginAction* > _exportFilterMap; /// List of all plugins loaded accessable by their export filter for faster lookup
		QHash<QString, CAPluginAction* > _importFilterMap; /// List of all plugins loaded accessable by their import filter for faster lookup
		CAMainWin *_mainWin;
		QString _curChars;
		QStack<QString> _tree;						      /// Hierarchy backtrack of the current node
		
		CAPlugin *_curPlugin;
		QString _curPluginCanorusVersion;
		QString _curPluginLocale;
		
			// <action> tag:
			QHash<QString, QString> _curActionText;     /// List of actions LOCALE texts
			QString _curActionName;
			QString _curActionLocale;                   /// Temporary lang
			QString _curActionOnAction;
			QHash<QString,QString> _curActionExportFilter, _curActionImportFilter;
			QString _curActionParentMenu, _curActionParentToolbar;
			QString _curActionLang, _curActionFunction, _curActionFilename;
			QList<QString> _curActionArgs;
		
			// <menu> tag:
			QString _curMenuName;
			QHash<QString, QString> _curMenuTitle;      /// List of menus LOCALE titles			
			QString _curMenuLocale;                     /// Temporary lang
};

#endif /*PLUGINMANAGER_H_*/
