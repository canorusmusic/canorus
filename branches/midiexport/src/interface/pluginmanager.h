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

class CAPluginManager : public QXmlDefaultHandler {
public:
	CAPluginManager(CAMainWin *mainWin, CAPlugin *plugin);
	~CAPluginManager();
	
	static void readPlugins();
	static bool enablePlugin(CAPlugin* plugin, CAMainWin *mainWin);
	static bool enablePlugins(CAMainWin *mainWin);		
	static bool disablePlugin(CAPlugin* plugin);
	static bool disablePlugins();
	
	static void action(QString onAction, CADocument *document, QEvent *evt, QPoint *coords, CAMainWin *mainWin);
	static bool exportFilterExists(const QString filter) { return _exportFilterMap.contains(filter); }
	static void exportAction(QString filter, CADocument *document, QString filename);
	static bool importFilterExists(const QString filter) { return _importFilterMap.contains(filter); }
	static void importAction(QString filter, CADocument *document, QString filename);
	
	static bool installPlugin(QString path);
	static bool removePlugin(CAPlugin* plugin);
	
	static const QList<CAPlugin*> pluginList() { return _pluginList; };
	
	// XML parser functions:
	bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName,
	                  const QXmlAttributes& attributes);
       bool endElement(const QString& namespaceURI, const QString& localName,
	                const QString& qName);
	bool fatalError(const QXmlParseException& exception);
	bool characters(const QString& ch);
	
private:
	static QList<CAPlugin*> _pluginList;
	static QMultiHash<QString, CAPlugin* > _actionMap;
	static QHash<QString, CAPluginAction* > _exportFilterMap;
	static QHash<QString, CAPluginAction* > _importFilterMap;
	
	// non-static members needed while parsing plugin's descriptor file:
	CAMainWin *_mainWin;
	QString _curChars;
	QStack<QString> _tree;
	
	CAPlugin *_curPlugin;
	QString _curPluginCanorusVersion;
	QString _curPluginLocale;
	
		// <action> tag:
		QHash<QString, QString> _curActionText;        // List of actions LOCALE texts
		QString _curActionName;
		QString _curActionLocale;                      // Temporary lang
		QString _curActionOnAction;
		QHash<QString,QString> _curActionExportFilter, _curActionImportFilter;
		QString _curActionParentMenu, _curActionParentToolbar;
		bool _curActionRefresh;
		QString _curActionLang, _curActionFunction, _curActionFilename;
		QList<QString> _curActionArgs;
	
		// <menu> tag:
		QString _curMenuName;
		QHash<QString, QString> _curMenuTitle;         // List of menus LOCALE titles			
		QString _curMenuLocale;                        // Temporary lang
		QString _curMenuParentMenu;                    // Parent menu of the menu
};
#endif /*PLUGINMANAGER_H_*/
