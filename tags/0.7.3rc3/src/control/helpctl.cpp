/*!
        Copyright (c) 2009, 2016 Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.

        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QStringList>
#include <QDockWidget>
#include <QDesktopServices>

#include "canorus.h"
#include "control/helpctl.h"
#include "ui/mainwin.h"

#ifdef QT_WEBENGINEWIDGETS_LIB
#include "widgets/helpbrowser.h"
#endif

/*!
	\class CAHelpCtl
	\brief Class for showing User's guide, What's new, Did you know tips etc.
 */

/*!
	Initializes Help and loads User's guide.
 */
CAHelpCtl::CAHelpCtl() {
	_homeUrl = detectHomeUrl();
}

CAHelpCtl::~CAHelpCtl() {
}

/*!
	Helper function which returns the preferred User's guide language.
 */
QUrl CAHelpCtl::detectHomeUrl() {
	QUrl url;
	QFileInfo i;
	
	i=QFileInfo("doc:usersguide2/build/"+QLocale::system().name()+"/index.html");
	
	if ( !i.exists() ) {
		i=QFileInfo("doc:usersguide2/"+QLocale::system().name()+"/index.html");
	}
	
	if ( !i.exists() ) {
		i=QFileInfo("doc:usersguide2/build/"+QLocale::system().name().left(2)+"/index.html");
	}
	
	if ( !i.exists() ) {
		i=QFileInfo("doc:usersguide2/"+QLocale::system().name().left(2)+"/index.html");
	}
	
	if ( !i.exists() ) {
		i=QFileInfo("doc:usersguide2/build/en/index.html");
	}

	if ( !i.exists() ) {
		i=QFileInfo("doc:usersguide2/en/index.html");
	}
	
	if ( i.exists() ) {
		url = QUrl::fromLocalFile(i.absoluteFilePath());
	}
	
	return url;
}

/*!
	Show user's guide at the given chapter.
    
	\return True, if a user's guide was found and shown; False otherwise.
 */
bool CAHelpCtl::showUsersGuide( QString chapter, QWidget *helpWidget ) {
	QUrl url = _homeUrl;
	
	if (!chapter.isEmpty()) {
		url.setFragment(chapter);
	}
	
	if (!url.path().isEmpty()) {
#ifdef QT_WEBENGINEWIDGETS_LIB
		displayHelp( url, helpWidget );
#else
		QDesktopServices::openUrl( url );
#endif
		return true;
	}
	
	return false;
}

/*!
	Activates the user's guide help at the given url.
 */
void CAHelpCtl::displayHelp( QUrl url, QWidget *helpWidget ) {
#ifdef QT_WEBENGINEWIDGETS_LIB
	CAHelpBrowser *browser=0;
	if ( !helpWidget ) {
		browser = new CAHelpBrowser;
		browser->setAttribute(Qt::WA_DeleteOnClose);
	} else
	if (dynamic_cast<CAMainWin*>(helpWidget)) {
		browser = static_cast<CAMainWin*>(helpWidget)->helpWidget();
		static_cast<CAMainWin*>(helpWidget)->helpDock()->show();
	}

	if (browser) {
		browser->setUrl( url );
	}
#endif
}
