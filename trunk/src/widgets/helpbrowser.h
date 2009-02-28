/*!
	Copyright (c) 2009, Itay Perl, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

    Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef HELPBROWSER_H_
#define HELPBROWSER_H_

#include <QTextBrowser>
class QHelpEngine;

class CAHelpBrowser : public QTextBrowser {
	Q_OBJECT
	public:
		CAHelpBrowser(QWidget* parent = 0, QHelpEngine *helpEngine = 0);
		~CAHelpBrowser() {}
		QVariant loadResource(int type, const QUrl& url);
		void setSource(const QUrl& url);
		inline void setHelpEngine(QHelpEngine* helpEngine) { _helpEngine = helpEngine; }
		inline QHelpEngine* helpEngine() { return _helpEngine; }
	private:
		QHelpEngine* _helpEngine;
};

#endif /* HELPBROWSER_H_ */

