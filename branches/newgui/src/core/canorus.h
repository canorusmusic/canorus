/** \file core/canorus.h
 * 
 * Copyright (c) 2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */
 
#ifndef CANORUS_H_
#define CANORUS_H_

#include <QString>
#include <QList>

#include "ui/mainwin.h"

class QSettings;

class CACanorus {
public:
	static void initMain();
	static void initSettings();
	static void parseAppArguments(int argc, char *argv[]);
	
	static QString locateResource(const QString fileName);
	static QString locateResourceDir(const QString fileName);	
	
	inline static int mainWinCount() { return _mainWinList.size(); }
	inline static CAMainWin* mainWinAt(int idx) { return _mainWinList[idx]; }
	inline static void removeMainWin(CAMainWin *w) { _mainWinList.removeAll(w); }
	inline static void addMainWin(CAMainWin *w, bool show=true) { _mainWinList << w; if (show) w->show(); }
	
	inline static QSettings *settings() { return _settings; }
	
private:
	static QList<CAMainWin*> _mainWinList;
	static QSettings *_settings;
};
#endif /* CANORUS_H_*/
