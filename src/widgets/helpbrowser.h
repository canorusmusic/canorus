/*!
	Copyright (c) 2009, Itay Perl, Canorus development team
    Copyright (c) 2016, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

    Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef HELPBROWSER_H_
#define HELPBROWSER_H_

#if QT_VERSION >= 0x050500
#include <QWebEngineView>
#else
#include <QWebView>
#endif

class CAHelpBrowser
#if QT_VERSION >= 0x050500
: public QWebEngineView
#else
: public QWebView
#endif
{
	Q_OBJECT
	public:
		CAHelpBrowser(QWidget* parent = 0);
		~CAHelpBrowser() {}
};

#endif /* HELPBROWSER_H_ */

