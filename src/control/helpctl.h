/*!
        Copyright (c) 2009, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.

        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef HELPCTL_H_
#define HELPCTL_H_

#include <QString>

class QProcess;

class CAHelpCtl {
public:
	CAHelpCtl();
	virtual ~CAHelpCtl();

	void showUsersGuide( QString chapter="" );

private:
	QProcess *_usersGuideProcess;
};

#endif /* HELPCTL_H_ */
