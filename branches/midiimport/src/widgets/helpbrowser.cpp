/*!
	Copyright (c) 2009, Itay Perl, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QHelpEngine>
#include <QDesktopServices>
#include "widgets/helpbrowser.h"


CAHelpBrowser::CAHelpBrowser(QWidget* parent, QHelpEngine *helpEngine) 
	: QTextBrowser(parent), _helpEngine(helpEngine)
{
}

/*!
	Open network URLs in an external browser, without changing the QTextBrowser source.
*/

void CAHelpBrowser::setSource(const QUrl& url)
{
	if (url.scheme() == "http" || url.scheme() == "https" || url.scheme() == "mailto")
		QDesktopServices::openUrl(url);
	else
		QTextBrowser::setSource(url);
}

/*!
	Handle qthelp URLs correctly.
*/

QVariant CAHelpBrowser::loadResource(int type, const QUrl& url) {
	if( url.scheme() == "qthelp" && _helpEngine ) {
		return QVariant(_helpEngine->fileData(url));
	} else {
		return QTextBrowser::loadResource(type, url);
	}
}
