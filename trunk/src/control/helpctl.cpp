/*!
        Copyright (c) 2009, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.

        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QStringList>
#include <QTextStream>
#include <QProcess>

#include "canorus.h"
#include "control/helpctl.h"

/*!
	\class CAHelpCtl
	\brief Class for showing User's guide, What's new, Did you know tips etc.
 */

CAHelpCtl::CAHelpCtl()
 : _usersGuideProcess(0) {

}

CAHelpCtl::~CAHelpCtl() {
}

void CAHelpCtl::showUsersGuide( QString chapter ) {
	if (!_usersGuideProcess || _usersGuideProcess->state()!=QProcess::Running) {
		if (_usersGuideProcess) delete _usersGuideProcess;

		QStringList location = CACanorus::locateResource(QString("doc/usersguide/")+QLocale::system().name()+".qhc");
		if (location.isEmpty()) {
			location = CACanorus::locateResource("doc/usersguide/en.qhc");
		}

		if (!location.isEmpty()) {
			_usersGuideProcess = new QProcess;
			QStringList args;
			args << "-collectionFile"
			    << location[0]
			    << "-enableRemoteControl";
			_usersGuideProcess->start("assistant", args);

			if (!_usersGuideProcess->waitForStarted()) {
				std::cout << "Error launching Qt assistant." << endl;
			}
		}
	}

	if (!_usersGuideProcess)
		return;

	/*if (chapter.isEmpty()) {
		QTextStream str(_usersGuideProcess);
		QStringList location = CACanorus::locateResource(QString("doc/usersguide/")+QLocale::system().name()+".qhc");
 		if (!location.isEmpty()) {
 			str << (QString("setSource qthelp://canorus/usersguide/")+QLocale::system().name()+".html") << endl;
 		} else {
 			str << "setSource qthelp://canorus/usersguide/en.html" << endl;
 		}
	}*/
}
