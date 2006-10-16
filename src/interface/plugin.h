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

class QPoint;
class QEvent;

class CAMainWin;
class CADocument;

class CAPlugin {
	public:
		CAPlugin();
		CAPlugin(QString name, QString author, QString version, QString date, QString dirName, QString homeUrl, QString updateUrl);
		~CAPlugin();
		
		/**
		 * This method is called when an event was triggered in a program and a plugin connected to this event is enabled.
		 * 
		 * @param onAction String describing action name. eg. onScoreViewPortClick, onDocumentClose
		 * @param mainWin Pointer to the current application main window.
		 * @param document Pointer to the current document.
		 * @param evt Pointer to the Mouse/Key/Wheel event, if it happened.
		 * @param coords Pointer to the coords in absolute world units, if a click on ScoreViewPort happened.
		 */
		bool action(QString onAction, CAMainWin *mainWin=0, CADocument *document=0, QEvent *evt=0, QPoint *coords=0);
		
		/**
		 * Add an action to the plugin.
		 * 
		 * @param onAction Name of the Canorus's action the plugin reacts on.
		 * @param lang Type of the action (ruby, python, library etc.).
		 * @param fileName File name of the script/library which is to be run on Canorus event.
		 * @param function Name of the function to be run.
		 * @param args List of arguments expected by the function.
		 */
		void addAction(QString onAction, QString lang, QString fileName, QString function, QList<QString> args);
		
		void setEnabled(bool enabled) { _enabled = enabled; }
		bool isEnabled() { return _enabled; }
		
		QString name() { return _name; }
		QString author() { return _author; }
		QString version() { return _version; }
		QString date() { return _date; }
		QString dirName() { return _dirName; }
		QString homeUrl() { return _homeUrl; }
		QString updateUrl() { return _updateUrl; }
		
		void setName(QString name, QString lang=QString("")) { if (lang=="") _name = name; else _nameMap[lang]=name; }
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
		QHash<QString, QString> _nameMap;	///LOCALE name of the plugin, key means the code, value is the translation
		QString _author;
		QString _version;
		QString _date;
		QString _dirName;	///ABSOLUTE Name of directory where the plugin should be extracted to - plugin only stores the relative one, Canorus changes it to absolute one
		QString _homeUrl;
		QString _updateUrl;
		bool _enabled;
		
		QHash< QString, QList<QString> > _actionMap;	///Canorus action name -> action type, action filename, action function name and a number types of arguments the external function needs
};

#endif /*PLUGIN_H_*/
