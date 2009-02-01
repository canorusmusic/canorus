/*!
        Copyright (c) 2009, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.

        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QStringList>
#include <QHelpEngine>
#include <QTextBrowser>
#include <QDockWidget>

#include "canorus.h"
#include "control/helpctl.h"
#include "ui/mainwin.h"

/*!
	\class CAHelpCtl
	\brief Class for showing User's guide, What's new, Did you know tips etc.
 */

CAHelpCtl::CAHelpCtl() {
	QStringList location = CACanorus::locateResource(QString("doc/usersguide/")+QLocale::system().name()+".qhc");
	if (location.isEmpty()) {
		location = CACanorus::locateResource("doc/usersguide/en.qhc");
	}

	if (!location.isEmpty()) {
		_helpEngine = new QHelpEngine(location[0]);
	}
}

CAHelpCtl::~CAHelpCtl() {
}

void CAHelpCtl::showUsersGuide( QString chapter, QWidget *helpWidget ) {
	QUrl url;

	if (chapter.isEmpty()) {
		QStringList location = CACanorus::locateResource(QString("doc/usersguide/")+QLocale::system().name()+".qhc");
		if (!location.isEmpty()) {
			url = QString("qthelp://canorus/usersguide/")+QLocale::system().name()+".html";
		} else {
			url = QString("qthelp://canorus/usersguide/en.html");
		}
	} else {
		QMap<QString, QUrl> links = _helpEngine->linksForIdentifier(chapter);
		if (links.count()) {
			url = links.constBegin().value();
		}
	}

	displayHelp( url, helpWidget );
}

void CAHelpCtl::displayHelp( QUrl url, QWidget *helpWidget ) {
	QByteArray help = _helpEngine->fileData(url);

	QTextEdit *textEdit=0;
	if ( !helpWidget ) {
		textEdit = new QTextBrowser();
		textEdit->setAttribute(Qt::WA_DeleteOnClose);
	} else
	if (dynamic_cast<CAMainWin*>(helpWidget)) {
		textEdit = static_cast<CAMainWin*>(helpWidget)->helpWidget();
		static_cast<CAMainWin*>(helpWidget)->helpDock()->show();
	}

	if (textEdit) {
		textEdit->setText( help );
	}
}
