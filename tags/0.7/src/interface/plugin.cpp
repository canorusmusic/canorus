/** @file interface/plugin.cpp
 *
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 *
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifdef USE_SWIG
// Python.h, which swigpython.h includes, must be included before any other headers
#include "scripting/swigpython.h"
#include "scripting/swigruby.h"
#endif

#include "interface/plugin.h"
#include "interface/pluginaction.h"

#ifndef SWIGCPP
#include "canorus.h"
#include "ui/mainwin.h"
#include "widgets/viewportcontainer.h"
#include "widgets/viewport.h"
#include "widgets/scoreviewport.h"
#include "drawable/drawablemuselement.h"
#else
#include <QMenu>
#include "interface/plugins_swig.h"
#endif

extern "C" void guiError(); // defined in canoruspython.i

CAPlugin::CAPlugin() {
	_name = "";
	_author = "";
	_version = "";
	_date = "";
	_dirName = "";
	_homeUrl = "";
	_updateUrl = "";

	_enabled = false;
}

CAPlugin::CAPlugin(QString name, QString author, QString version, QString date, QString dirName, QString homeUrl, QString updateUrl) {
	_name = name;
	_author = author;
	_version = version;
	_date = date;
	_dirName = dirName;
	_homeUrl = homeUrl;
	_updateUrl = updateUrl;

	_enabled = false;
}

CAPlugin::~CAPlugin() {
	QList<CAPluginAction*> pluginActions = _actionMap.values();
	for (int i=0; i<pluginActions.size(); i++)
		delete pluginActions[i];

	QList<QMenu*> menus = _menuMap.values();
	for (int i=0; i<menus.size(); i++)
		delete menus[i];
}

bool CAPlugin::action(QString onAction, CAMainWin *mainWin, CADocument *document, QEvent *evt, QPoint *coords) {
	if (!_enabled)
		return false;

	QList<CAPluginAction*> actionList = _actionMap.values(onAction);
	if (!actionList.size())	// action not found
		return false;

	bool error = false;
	for (int i=0; i<actionList.size(); i++)
		error |= (!callAction(actionList[i], mainWin, document, evt, coords));

	return (!error);
}

bool CAPlugin::callAction(CAPluginAction *action, CAMainWin *mainWin, CADocument *document, QEvent *evt, QPoint *coords, QString filename) {
	bool error=false;
#ifdef USE_RUBY
	QList<VALUE> rubyArgs;
#endif
#ifdef USE_PYTHON
	QList<PyObject*> pythonArgs;
#endif

	bool rebuildDocument = false;

	// Convert arguments to its needed scripting language types
	QList<QString> args = action->args();
	for (int i=0; i<args.size(); i++) {
		QString val=args[i];

		// Currently selected document
		if (val=="document") {
			rebuildDocument = true;
#ifdef USE_RUBY
			if (action->lang()=="ruby") {
				rubyArgs << CASwigRuby::toRubyObject(document, CASwigRuby::Document);
			}
#endif
#ifdef USE_PYTHON
			if (action->lang()=="python") {
				pythonArgs << CASwigPython::toPythonObject(document, CASwigPython::Document);
			}
#endif
		} else

		// Currently selected sheet
		if (val=="sheet") {
#ifdef USE_RUBY
			if (action->lang()=="ruby") {
				if ( mainWin->currentSheet() )
					rubyArgs << CASwigRuby::toRubyObject( mainWin->currentSheet(), CASwigRuby::Sheet );
				else {
					error = true;
					break;
				}
			}
#endif
#ifdef USE_PYTHON
			if (action->lang()=="python") {
				if ( mainWin->currentSheet() )
					pythonArgs << CASwigPython::toPythonObject( mainWin->currentSheet(), CASwigPython::Sheet );
				else {
					error = true;
					break;
				}
			}
#endif
		} else

		// Currently selected note
		if (val=="note") {
#ifdef USE_RUBY
			if (action->lang()=="ruby") {
#ifndef SWIGCPP
				if ( mainWin->currentScoreViewPort() ) {
					CAScoreViewPort *v = mainWin->currentScoreViewPort();
					if (!v->selection().size() || v->selection().front()->drawableMusElementType()!=CADrawableMusElement::DrawableNote) {
						error=true;
						break;
					}
					rubyArgs << CASwigRuby::toRubyObject(v->selection().front()->musElement(), CASwigRuby::Note);
				}
				else {
					error = true;
					break;
				}
#endif
			}
#endif
#ifdef USE_PYTHON
			if (action->lang()=="python") {
#ifndef SWIGCPP
				if ( mainWin->currentScoreViewPort() ) {
					CAScoreViewPort *v = mainWin->currentScoreViewPort();
					if (!v->selection().size() || v->selection().front()->drawableMusElementType()!=CADrawableMusElement::DrawableNote) {
						error=true;
						break;
					}
					pythonArgs << CASwigPython::toPythonObject(v->selection().front()->musElement(), CASwigPython::MusElement);
				}
				else {
					error = true;
					break;
				}
#endif
			}
#endif
		} else
		if (val=="chord") {
			//TODO
		} else

		// Selection in the current score view
		if (val=="selection") {
#ifdef USE_PYTHON
			if (action->lang()=="python") {
#ifndef SWIGCPP
				if ( mainWin->currentScoreViewPort() ) {
					QList<CAMusElement*> musElements = mainWin->currentScoreViewPort()->musElementSelection();
					PyObject *list = PyList_New(0);
					for (int i=0; i<musElements.size(); i++) {
						PyList_Append(list, CASwigPython::toPythonObject(musElements[i], CASwigPython::MusElement));
					}

					pythonArgs << list;
				}
				else {
					error = true;
					break;
				}
#endif
			}
#endif
		} else

		// Directory of the plugin
		if (val=="pluginDir") {
#ifdef USE_RUBY
			if (action->lang()=="ruby") {
				rubyArgs << CASwigRuby::toRubyObject(&_dirName, CASwigRuby::String);
			}
#endif
#ifdef USE_PYTHON
			if (action->lang()=="python") {
				pythonArgs << CASwigPython::toPythonObject(&_dirName, CASwigPython::String);
			}
#endif
		} else

		// File name selected in export/import dialogs
		if (val=="export-filename" || val=="import-filename") {
#ifdef USE_RUBY
			if (action->lang()=="ruby") {
				rubyArgs << CASwigRuby::toRubyObject(&filename, CASwigRuby::String);
			}
#endif
#ifdef USE_PYTHON
			if (action->lang()=="python") {
				pythonArgs << CASwigPython::toPythonObject(&filename, CASwigPython::String);
			}
#endif
		}
	}
#ifdef USE_PYTHON
	if (_name == "pyCLI") {
		if (mainWin->pyConsoleIface)
			pythonArgs << CASwigPython::toPythonObject(mainWin->pyConsoleIface, CASwigPython::PyConsoleInterface);
	}
#endif

	//add the plugin's path for the first time, so scripting languages can find their modules
	if (action->onAction()=="onInit") {
#ifdef USE_RUBY
		if (action->lang()=="ruby") {
			rb_eval_string((QString("$: << '") + dirName() + "'").toStdString().c_str());
		}
#endif
#ifdef USE_PYTHON
		if (action->lang()=="python") {
			PyRun_SimpleString((QString("sys.path.append('")+dirName()+"')").toStdString().c_str());
		}
#endif
	}

	if (!error) {
#ifdef USE_RUBY
		if (action->lang()=="ruby") {
			error = (!CASwigRuby::callFunction(_dirName + "/" + action->filename(), action->function(), rubyArgs));
		}
#endif
#ifdef USE_PYTHON
		if (action->lang()=="python") {
			error = (!CASwigPython::callFunction(_dirName + "/" + action->filename(), action->function(), pythonArgs));
		}
#endif
	}

#ifndef SWIGCPP
	if (action->refresh()) {
		if (rebuildDocument)
			CACanorus::rebuildUI(document);
		else
			CACanorus::rebuildUI(document, mainWin->currentSheet());
	}
#endif

	return (!error);
}

void CAPlugin::addAction(CAPluginAction *action) {
	if (!_actionMap.values(action->onAction()).contains(action))
		_actionMap.insertMulti(action->onAction(), action);
}
