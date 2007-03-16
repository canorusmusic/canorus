/** @file interface/plugin.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifdef USE_SWIG
#include "core/canorus.h"
#include "scripting/swigruby.h"
#include "scripting/swigpython.h"
#endif

#include "interface/plugin.h"
#include "ui/pluginaction.h"

#include "ui/mainwin.h"
#include "widgets/viewportcontainer.h"
#include "widgets/viewport.h"
#include "widgets/scoreviewport.h"
#include "drawable/drawablemuselement.h"

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
				if (mainWin->currentViewPortContainer() && mainWin->currentViewPortContainer()->lastUsedViewPort() && mainWin->currentViewPortContainer()->lastUsedViewPort()->viewPortType()==CAViewPort::ScoreViewPort)
					rubyArgs << CASwigRuby::toRubyObject(((CAScoreViewPort*)mainWin->currentViewPortContainer()->lastUsedViewPort())->sheet(), CASwigRuby::Sheet);
				else {
					error = true;
					break;
				}
			}
#endif
#ifdef USE_PYTHON
			if (action->lang()=="python") {
				if (mainWin->currentViewPortContainer() && mainWin->currentViewPortContainer()->lastUsedViewPort() && mainWin->currentViewPortContainer()->lastUsedViewPort()->viewPortType()==CAViewPort::ScoreViewPort)
					pythonArgs << CASwigPython::toPythonObject(static_cast<CAScoreViewPort*>(mainWin->currentViewPortContainer()->lastUsedViewPort())->sheet(), CASwigPython::Sheet);
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
				if (mainWin->currentViewPortContainer()->lastUsedViewPort()->viewPortType()==CAViewPort::ScoreViewPort) {
					CAScoreViewPort *v = (CAScoreViewPort*)(mainWin->currentViewPortContainer()->lastUsedViewPort());
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
			}
#endif
#ifdef USE_PYTHON
			if (action->lang()=="python") {
				if (mainWin->currentViewPortContainer()->lastUsedViewPort()->viewPortType()==CAViewPort::ScoreViewPort) {
					CAScoreViewPort *v = static_cast<CAScoreViewPort*>(mainWin->currentViewPortContainer()->lastUsedViewPort());
					if (!v->selection().size() || v->selection().front()->drawableMusElementType()!=CADrawableMusElement::DrawableNote) {
						error=true;
						break;
					}
					pythonArgs << CASwigPython::toPythonObject(v->selection().front()->musElement(), CASwigPython::Note);
				}
				else {
					error = true;
					break;
				}
			}
#endif
		} else
		if (val=="chord") {
			//TODO
		} else
		
		// Directory of the plugin
		if (val=="pluginDir") {
#ifdef USE_RUBY
			if (action->lang()=="ruby") {
				rubyArgs << CASwigRuby::toRubyObject(new QString(dirName()), CASwigRuby::String);
			}
#endif
#ifdef USE_PYTHON
			if (action->lang()=="python") {
				pythonArgs << CASwigPython::toPythonObject(new QString(dirName()), CASwigPython::String);
			}
#endif	
		}
		
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
	
	if (action->refresh()) {
		if (rebuildDocument)
			CACanorus::rebuildUI(document);
		else
			CACanorus::rebuildUI(document, mainWin->currentSheet());
	}
	
	return (!error);
}

void CAPlugin::addAction(CAPluginAction *action) {
	if (!_actionMap.values(action->onAction()).contains(action))
		_actionMap.insertMulti(action->onAction(), action);
}
