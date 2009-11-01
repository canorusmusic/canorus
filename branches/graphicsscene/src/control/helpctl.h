/*!
        Copyright (c) 2009, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.

        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef HELPCTL_H_
#define HELPCTL_H_

#include <QString>
#include <QUrl>

class QHelpEngine;
class QWidget;

class CAHelpCtl {
public:
	CAHelpCtl();
	virtual ~CAHelpCtl();

	void showUsersGuide( QString chapter="", QWidget *helpWidget=0 );

private:
	void displayHelp( QUrl url, QWidget *helpWidget );
	QString usersGuideLanguage();

	QHelpEngine *_helpEngine;
};

#endif /* HELPCTL_H_ */
