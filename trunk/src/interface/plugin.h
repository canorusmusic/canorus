/*!
	Copyright (c) 2006, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef PLUGIN_H_
#define PLUGIN_H_

#include <QString>
#include <QHash>
#include <QList>
#include <QLocale>

class QPoint;
class QEvent;
class QMenu;

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
		 * @param onAction String describing action name. eg. onScoreClick, onDocumentClose
		 * @param mainWin Pointer to the current application main window.
		 * @param document Pointer to the current document.
		 * @param evt Pointer to the Mouse/Key/Wheel event, if it happened.
		 * @param coords Pointer to the coords in absolute world units, if a click on Score happened.
		 * @return True, if all the actions were successfully called, False otherwise.
		 */
		bool action(QString onAction, CAMainWin *mainWin=0, CADocument *document=0, QEvent *evt=0, QPoint *coords=0);

		/**
		 * This function calls a specific action. This is used for export, import and custom actions which aren't called by Canorus automatically.
		 * @param action Pointer to the action to be called.
		 * @param mainWin Pointer to the current application main window.
		 * @param document Pointer to the current document.
		 * @param evt Pointer to the Mouse/Key/Wheel event, if it happened.
		 * @param coords Pointer to the coords in absolute world units, if a click on Score happened.
		 * @return True, if the action succeeded, False otherwise.
		 */
		bool callAction(CAPluginAction *action, CAMainWin *mainWin=0, CADocument *document=0, QEvent *evt=0, QPoint *coords=0, QString filename="");

		/**
		 * Adds an action to the plugin, if the actionName&action aren't present yet.
		 *
		 * @param onAction Name of the Canorus's action the plugin reacts on.
		 * @param lang Type of the action (ruby, python, library etc.).
		 * @param fileName File name of the script/library which is to be run on Canorus event.
		 * @param function Name of the function to be run.
		 * @param args List of arguments expected by the function.
		 */
		void addAction(CAPluginAction *action);
		void addMenu(QString name, QMenu* menu) { _menuMap[name] = menu; }

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
		QMenu *menu(QString menuName) { return _menuMap[menuName]; }

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
		QHash<QString, QString> _descMap;                   /// LOCALE description of the plugin, key means the language code, value is the translation. Empty string key is the key of universal non-translated description.
		QString _author;
		QString _version;
		QString _date;
		QString _dirName;                                   /// ABSOLUTE Name of directory where the plugin is located - plugin only stores the relative one, Canorus changes it to absolute one
		QString _homeUrl;
		QString _updateUrl;
		bool _enabled;

		QMultiHash< QString, CAPluginAction* > _actionMap;  /// Key: onAction, Value: plugin's action
		QHash< QString, QMenu* > _menuMap;                  /// Map of plugin menu name -> Canorus menu object
};
#endif /*PLUGIN_H_*/
