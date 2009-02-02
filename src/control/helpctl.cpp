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

/*!
	Initializes Help and loads User's guide.
 */
CAHelpCtl::CAHelpCtl()
 : _helpEngine(0) {
	QString lang = usersGuideLanguage();

	if (!lang.isEmpty()) {
		_helpEngine = new QHelpEngine( CACanorus::locateResource(QString("doc/usersguide/")+lang+".qhc")[0] );
	}
}

CAHelpCtl::~CAHelpCtl() {
}

/*!
	Helper function which returns the existent User's guide language.
 */
QString CAHelpCtl::usersGuideLanguage() {
	if (CACanorus::locateResource(QString("doc/usersguide/")+QLocale::system().name()+".qhc").size()) {
		return QLocale::system().name();
	} else
	if (CACanorus::locateResource(QString("doc/usersguide/")+QLocale::system().name().left(2)+".qhc").size()) {
		return QLocale::system().name().left(2);
	} else
	if (CACanorus::locateResource(QString("doc/usersguide/en.qhc")).size()) {
		return "en";
	} else {
		return "";
	}
}

/*!
	Loads user's guide file
 */
void CAHelpCtl::showUsersGuide( QString chapter, QWidget *helpWidget ) {
	QUrl url;

	if (!_helpEngine) {
		return;
	}

	if (chapter.isEmpty()) {
		if (!usersGuideLanguage().isEmpty()) {
			url = QString("qthelp://canorus/usersguide-")+usersGuideLanguage()+"/"+usersGuideLanguage()+".html";
		} else {
			return;
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
