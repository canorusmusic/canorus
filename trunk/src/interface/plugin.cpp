/** @file interface/plugin.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifdef USE_SWIG
#include "scripting/swigruby.h"
#include "scripting/swigpython.h"
#endif

#include "interface/plugin.h"

#include "ui/mainwin.h"
#include "widgets/scrollwidget.h"
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
}

bool CAPlugin::action(QString actionName, CAMainWin *mainWin, CADocument *document, QEvent *evt, QPoint *coords) {
	if (!_enabled)
		return false;
	
	QList<QString> vals = _actionMap[actionName];
	//vals[0] - type
	//vals[1] - file name
	//vals[2] - function name
	//vals[3+] - arguments
	
	int i=0;
	QString lang = vals[i++].toUpper();
	QString fileName = vals[i++];
	QString functionName = vals[i++];
	
#ifdef USE_RUBY
	QList<VALUE> rubyArgs;
#endif
#ifdef USE_PYTHON
	QList<PyObject*> pythonArgs;
#endif
	bool error=false;
	for (; i<vals.size(); i++) {
		QString val=vals[i].toUpper();
		if (val=="DOCUMENT") {
#ifdef USE_RUBY
			if (lang=="RUBY") {
				rubyArgs << CASwigRuby::toRubyObject(document, CASwigRuby::Document);
			}
#endif
#ifdef USE_PYTHON
			if (lang=="PYTHON") {
				pythonArgs << CASwigPython::toPythonObject(document, CASwigPython::Document);
			}
#endif
		} else
		if (val=="SHEET") {
#ifdef USE_RUBY
			if (lang=="RUBY") {
				if (mainWin->currentScrollWidget() && mainWin->currentScrollWidget()->lastUsedViewPort() && mainWin->currentScrollWidget()->lastUsedViewPort()->viewPortType()==CAViewPort::ScoreViewPort)
					rubyArgs << CASwigRuby::toRubyObject(((CAScoreViewPort*)mainWin->currentScrollWidget()->lastUsedViewPort())->sheet(), CASwigRuby::Sheet);
				else {
					error = true;
					break;
				}
			}
#endif
#ifdef USE_PYTHON
			if (lang=="PYTHON") {
				if (mainWin->currentScrollWidget() && mainWin->currentScrollWidget()->lastUsedViewPort() && mainWin->currentScrollWidget()->lastUsedViewPort()->viewPortType()==CAViewPort::ScoreViewPort)
					pythonArgs << CASwigPython::toPythonObject(((CAScoreViewPort*)mainWin->currentScrollWidget()->lastUsedViewPort())->sheet(), CASwigPython::Sheet);
				else {
					error = true;
					break;
				}
			}
#endif
		} else
		if (val=="NOTE") {
#ifdef USE_RUBY
			if (lang=="RUBY") {
				if (mainWin->currentScrollWidget()->lastUsedViewPort()->viewPortType()==CAViewPort::ScoreViewPort) {
					CAScoreViewPort *v = (CAScoreViewPort*)(mainWin->currentScrollWidget()->lastUsedViewPort());
					if (!v->selection()->size() || v->selection()->front()->drawableMusElementType()!=CADrawableMusElement::DrawableNote) {
						error=true;
						break;
					}
					rubyArgs << CASwigRuby::toRubyObject(v->selection()->front()->musElement(), CASwigRuby::Note);
				}
				else {
					error = true;
					break;
				}
			}
#endif
#ifdef USE_PYTHON
			if (lang=="PYTHON") {
				if (mainWin->currentScrollWidget()->lastUsedViewPort()->viewPortType()==CAViewPort::ScoreViewPort) {
					CAScoreViewPort *v = (CAScoreViewPort*)(mainWin->currentScrollWidget()->lastUsedViewPort());
					if (!v->selection()->size() || v->selection()->front()->drawableMusElementType()!=CADrawableMusElement::DrawableNote) {
						error=true;
						break;
					}
					pythonArgs << CASwigPython::toPythonObject(v->selection()->front()->musElement(), CASwigPython::Note);
				}
				else {
					error = true;
					break;
				}
			}
#endif
		} else
		if (val=="CHORD") {
			//TODO
		}
	}
	
	if (actionName=="onInit") {
		//add the plugin's path for the first time, so scripting languages can find their modules
#ifdef USE_RUBY
		if (lang=="RUBY") {
			rb_eval_string((QString("$: << '") + _dirName + "'").toStdString().c_str());
		}
#endif
#ifdef USE_PYTHON
		if (lang=="PYTHON") {
			PyRun_SimpleString((QString("sys.path.append('")+_dirName+"')").toStdString().c_str());
		}
#endif
	}
	
	if (!error) {
#ifdef USE_RUBY
		if (lang=="RUBY") {
			error = (!CASwigRuby::callFunction(_dirName + "/" + fileName, functionName, rubyArgs));
		}
#endif
#ifdef USE_PYTHON
		if (lang=="PYTHON") {
			error = (!CASwigPython::callFunction(_dirName + "/" + fileName, functionName, pythonArgs));
		}
#endif
	}
	
	return (!error);
}

void CAPlugin::addAction(QString actionName, QString type, QString fileName, QString function, QList<QString> args) {
	QList<QString> vals;
	vals << type;
	vals << fileName;
	vals << function;
	vals << args;
	
	_actionMap.insert(actionName, vals);
}
