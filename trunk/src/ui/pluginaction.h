/** @file ui/pluginaction.h
 * 
 * Copyright (c) 2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef PLUGINACTION_H_
#define PLUGINACTION_H_

#include <QAction>

#include "interface/plugin.h"

/**
 * CAPluginAction class represents each <action> stanza found in Canorus Plugin descriptor XML file.
 * Every action has its unique name, scripting language, function, its arguments and filename.
 * Optional properties are Canorus internal action which it reacts on, export filter, menu/toolbar text.
 */
class CAPluginAction : public QAction {
	Q_OBJECT
public:
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
	inline QString localeText(QString lang) { return _text[lang]; }
	QString localText() { if (_text.contains(QLocale::system().name())) return localeText(QLocale::system().name()); else return localeText(""); }
	bool refresh() { return _refresh; }
	
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
	void setLocaleText(QString lang, QString value) { _text[lang] = value; 	this->setText(localText()); } 
	void setTexts(QHash< QString, QString > t) { _text = t;	this->setText(localText()); }
	void setRefresh(bool refresh) { _refresh = refresh; }
	
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
	bool _refresh;                         /// Should the UI be rebuilt when calling the action.

private slots:
	void triggeredSlot(bool);              /// Connected to triggered(), calls plugin->callAction()
	
signals:
	void triggered(QAction*, bool);        /// When the action is triggered, this signal is emitted
};

#endif /* PLUGINACTION_H_ */
