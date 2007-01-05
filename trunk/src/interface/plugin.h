/** @file interface/plugin.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef PLUGIN_H_
#define PLUGIN_H_

#include <QString>
#include <QHash>
#include <QList>
#include <QLocale>

class QPoint;
class QEvent;

class CAMainWin;
class CADocument;

class CAPluginAction;

class CAPlugin {
	public:
		CAPlugin();
		CAPlugin(QString name, QString author, QString version, QString date, QString dirName, QString homeUrl, QString updateUrl);
		~CAPlugin();
		
		/**
		 * This function is called when an event was triggered in a program and a plugin connected to this event is triggered.
		 * It calls all the plugin's actions having the given onAction.
		 * 
		 * @param onAction String describing action name. eg. onScoreViewPortClick, onDocumentClose
		 * @param mainWin Pointer to the current application main window.
		 * @param document Pointer to the current document.
		 * @param evt Pointer to the Mouse/Key/Wheel event, if it happened.
		 * @param coords Pointer to the coords in absolute world units, if a click on ScoreViewPort happened.
		 * @return True, if all the actions were successfully called, False otherwise.
		 */
		bool action(QString onAction, CAMainWin *mainWin=0, CADocument *document=0, QEvent *evt=0, QPoint *coords=0);
		
		/**
		 * This function calls a specific action. This is used for export, import and custom actions which aren't called by Canorus automatically.
		 * @param action Pointer to the action to be called.
		 * @param mainWin Pointer to the current application main window.
		 * @param document Pointer to the current document.
		 * @param evt Pointer to the Mouse/Key/Wheel event, if it happened.
		 * @param coords Pointer to the coords in absolute world units, if a click on ScoreViewPort happened.
		 * @return True, if the action succeeded, False otherwise.
		 */
		bool callAction(CAPluginAction *action, CAMainWin *mainWin=0, CADocument *document=0, QEvent *evt=0, QPoint *coords=0);
		
		/**
		 * Add an action to the plugin.
		 * 
		 * @param onAction Name of the Canorus's action the plugin reacts on.
		 * @param lang Type of the action (ruby, python, library etc.).
		 * @param fileName File name of the script/library which is to be run on Canorus event.
		 * @param function Name of the function to be run.
		 * @param args List of arguments expected by the function.
		 */
		void addAction(CAPluginAction *action);
		
		void setEnabled(bool enabled) { _enabled = enabled; }
		bool isEnabled() { return _enabled; }
		
		QString name() { return _name; }
		QString author() { return _author; }
		QString version() { return _version; }
		QString date() { return _date; }
		QString dirName() { return _dirName; }
		QString homeUrl() { return _homeUrl; }
		QString updateUrl() { return _updateUrl; }
		QString description(QString lang) { return _descMap[lang]; }
		QString localDescription() { if (_descMap.contains(QLocale::system().name())) return description(QLocale::system().name()); else return description(""); }
		
		void setName(QString name) { _name = name; }
		void setDescription(QString desc, QString lang=QString("")) { _descMap[lang]=desc; }
		void setAuthor(QString author) { _author = author; }
		void setVersion(QString version) { _version = version; }
		void setDate(QString date) { _date = date; }
		void setDirName(QString dirName) { _dirName = dirName; }
		void setHomeUrl(QString homeUrl) { _homeUrl = homeUrl; }
		void setUpdateUrl(QString updateUrl) { _updateUrl = updateUrl; }
		
		/**
		 * Return a list of all the actions the plugin is connected to.
		 * 
		 * @return List of actions in QList QString format.
		 */
		QList<QString> actionList() { return _actionMap.keys(); }

	private:
		QString _name;
		QHash<QString, QString> _descMap;	/// LOCALE description of the plugin, key means the language code, value is the translation. Empty string key is the key of universal non-translated description.
		QString _author;
		QString _version;
		QString _date;
		QString _dirName;	/// ABSOLUTE Name of directory where the plugin is located - plugin only stores the relative one, Canorus changes it to absolute one
		QString _homeUrl;
		QString _updateUrl;
		bool _enabled;
		
		QMultiHash< QString, CAPluginAction* > _actionMap;	/// Key: onAction, Value: plugin's action
};

/**
 * CAPluginAction class represents each <action> stanza found in Canorus Plugin descriptor XML file.
 * Every action has its unique name, scripting language, function, its arguments and filename.
 * Optional properties are Canorus internal action which it reacts on, export filter, menu/toolbar text.
 */
class CAPluginAction {
	public:
		/** Default constructor */
		CAPluginAction(CAPlugin *plugin, QString name, QString lang, QString function, QList<QString> args, QString filename);
		
		inline CAPlugin *plugin() { return _plugin; }
		inline QString name() { return _name; }
		inline QString lang() { return _lang; }
		inline QString function() { return _function; }
		inline QList<QString> args() { return _args; }
		inline QString filename() { return _filename; }
		inline QString onAction() { return _onAction; }
		inline QString exportFilter(QString lang) { return _exportFilter[lang]; }
		QString localExportFilter() { if (_exportFilter.contains(QLocale::system().name())) return exportFilter(QLocale::system().name()); else return exportFilter(""); }
		inline QString importFilter(QString lang) { return _importFilter[lang]; }
		QString localImportFilter() { if (_importFilter.contains(QLocale::system().name())) return exportFilter(QLocale::system().name()); else return importFilter(""); }
		inline QString text(QString lang) { return _text[lang]; }
		QString localText() { if (_text.contains(QLocale::system().name())) return text(QLocale::system().name()); else return text(""); }
		
		void setPlugin(CAPlugin *plugin) { _plugin = plugin; }
		void setName(QString name) { _name = name; }
		void setLang(QString lang) { _lang = lang; }
		void setFunction(QString function) { _function = function; }
		void addArgument(QString arg) { _args << arg; }
		bool removeArgument(QString arg) { _args.removeAll(arg); }
		void setFilename(QString filename) { _filename = filename; }
		void setOnAction(QString onAction) { _onAction = onAction; }
		void setExportFilter(QString lang, QString value) { _exportFilter[lang] = value; }
		void setExportFilters(QHash< QString, QString > f) { _exportFilter = f; }
		void setImportFilter(QString lang, QString value) { _importFilter[lang] = value; }
		void setImportFilters(QHash< QString, QString > f) { _importFilter = f; }
		void setText(QString lang, QString value) { _text[lang] = value; } 
		void setTexts(QHash< QString, QString > t) { _text = t; }
		
	private:
		CAPlugin *_plugin;                     /// Pointer to the plugin which this action belongs to
		QString _name;                         /// Action name
		QString _lang;                         /// Scripting language
		QString _function;                     /// Function name
		QList<QString> _args;                  /// Function arguments
		QString _filename;                     /// Filename which has the function
		QString _onAction;                     /// Canorus internal action which this action reacts on
		QHash<QString, QString> _exportFilter; /// Text written in export dialog's filter
		QHash<QString, QString> _importFilter; /// Text written in import dialog's filter
		QHash<QString, QString> _text;         /// Text written on a menu item or the toolbar button
//		QPixmap _icon;                         /// Menu/Toolbar button icon
};
#endif /*PLUGIN_H_*/
