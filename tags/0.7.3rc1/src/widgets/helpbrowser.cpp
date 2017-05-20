/*!
	Copyright (c) 2009, Itay Perl, Canorus development team
    Copyright (c) 2016, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "widgets/helpbrowser.h"
#include <QFile>
#include <QTextStream>

CAHelpBrowser::CAHelpBrowser(QWidget* parent) 
#if QT_VERSION >= 0x050600
	: QWebEngineView(parent)
#else
	: QWebView(parent)
#endif
{
}
