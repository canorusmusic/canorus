/** @file interface/plugin.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include "interface/plugin.h"
#include "scripting/swigruby.h"
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
	
	QList<VALUE> rubyArgs;
	bool error=false;
	for (; i<vals.size(); i++) {
		QString val=vals[i].toUpper();
		if (val=="COORDS") {
			if (lang=="RUBY") {
				//TODO: Convert C++ Qt's QPoint -> Ruby Qt's QPoint
			}
		} else
		if (val=="DOCUMENT") {
			if (lang=="RUBY") {
				rubyArgs << toRubyObject(document, CASwigRuby::Document);
			}
		} else
		if (val=="SHEET") {
			if (lang=="RUBY") {
				if (mainWin->currentScrollWidget()->lastUsedViewPort()->viewPortType()==CAViewPort::ScoreViewPort)
					rubyArgs << toRubyObject(((CAScoreViewPort*)mainWin->currentScrollWidget()->lastUsedViewPort())->sheet(), CASwigRuby::Sheet);
				else {
					error = true;
					break;
				}
			}
		} else
		if (val=="NOTE") {
			if (lang=="RUBY") {
				if (mainWin->currentScrollWidget()->lastUsedViewPort()->viewPortType()==CAViewPort::ScoreViewPort) {
					CAScoreViewPort *v = (CAScoreViewPort*)(mainWin->currentScrollWidget()->lastUsedViewPort());
					if (!v->selection()->size() || v->selection()->front()->drawableMusElementType()!=CADrawableMusElement::DrawableNote) {
						error=true;
						break;
					}
					rubyArgs << toRubyObject(v->selection()->front()->musElement(), CASwigRuby::Note);
				}
				else {
					error = true;
					break;
				}				
			}
		}
	}
	
	if (!error) {
		if (lang=="RUBY") {
			error = (!CASwigRuby::callFunction(_dirName + "/" + fileName, functionName, rubyArgs));
		}
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
